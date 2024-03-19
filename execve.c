/*
 * Copyright 2020-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <regex.h>

#include <unistd.h>

#include "iamroot.h"

typedef struct {
	regex_t re;
#ifndef JIMREGEXP_H
	char jimpad[40];
#endif
} __regex_t;

extern int __ldso_execve(const char *, char * const[], char * const[]);

static regex_t *re_ignore;

static void __regex_perror(const char *s, regex_t *regex, int err)
{
	char buf[128];
	regerror(err, regex, buf, sizeof(buf));
	if (!s) {
		dprintf(STDERR_FILENO, "%s\n", buf);
		return;
	}

	dprintf(STDERR_FILENO, "%s: %s\n", s, buf);
}

__attribute__((constructor,visibility("hidden")))
void execve_init()
{
	static __regex_t regex_ignore;
	const char *ignore;
	int ret;

#ifdef __linux__
	if (__secure())
		__warning("%s: secure-execution mode\n", __execfn());
#endif

	if (re_ignore)
		return;

	ignore = _getenv("IAMROOT_EXEC_IGNORE");
	if (!ignore)
		ignore = "mountpoint";

	ret = regcomp(&regex_ignore.re, ignore, REG_NOSUB|REG_EXTENDED);
	if (ret == -1) {
		__regex_perror("regcomp", &regex_ignore.re, ret);
		return;
	}

	re_ignore = &regex_ignore.re;
}

__attribute__((destructor,visibility("hidden")))
void execve_fini()
{
	if (!re_ignore)
		return;

	regfree(re_ignore);
	re_ignore = NULL;
}

static int ignore(const char *path)
{
	int ret = 0;

	if (!re_ignore)
		return 0;

	ret = regexec(re_ignore, path, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re_ignore, ret);
		return 0;
	}

	return !ret;
}

__attribute__((visibility("hidden")))
int __exec_ignored(const char *path)
{
	return ignore(path);
}

static int (*sym)(const char *, char * const[], char * const[]);

__attribute__((visibility("hidden")))
int next_execve(const char *path, char * const argv[], char * const envp[])
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "execve");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, argv, envp);
}

int execve(const char *path, char * const argv[], char * const envp[])
{
	char *interparg[14+1] = { NULL }; /*  0 ARGV0
					   *  1 /lib/ld.so
					   *  2 LD_LINUX_ARGV1
					   *  3 --preload
					   *  4 libiamroot.so:$LD_PRELOAD
					   *  5 --library-path
					   *  6 /usr/lib:/lib
					   *  7 --argv0
					   *  8 ARGV0
					   *  9 --inhibit-rpath
					   * 10 --inhibit-cache
					   * 11 /usr/lib/lib.so:/lib/lib.so
					   * 12 /bin/sh
					   * 13 -x
					   * 14 script.sh
					   * 15 NULL-terminated
					   */
	/*
	 * According to man execve(2):
	 *
	 * Interpreter scripts
	 *
	 * The kernel imposes a maximum length on the text that follows the
	 * "#!" characters at the start of a script; characters beyond the
	 * limit are ignored. Before Linux 5.1, the limit is 127 characters.
	 * Since Linux 5.1, the limit is 255 characters.
	 */
	/* See https://www.in-ulm.de/~mascheck/various/shebang/#results */
	char *hashbang = NULL, *program = NULL;
	char buf[2*PATH_MAX];
	char * const *arg;
	int argc, i, ret;
	off_t off = 0;
	ssize_t siz;

	/* Run exec.sh script */
	if (__exec_ignored(path))
		goto exec_sh;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return -1;
	program = buf;
	off += siz+1; /* NULL-terminated */

	__debug("%s(path: '%s' -> '%s', argv: { '%s', '%s', ... }, envp: %p)\n",
		__func__, path, buf, argv[0], argv[1], envp);

	interparg[0] = *argv; /* original argv0 as argv0 */

	/*
	 * In secure-execution mode, preload pathnames containing slashes are
	 * ignored. Furthermore, shared objects are preloaded only from the
	 * standard search directories and only if they have set-user-ID mode
	 * bit enabled (which is not typical).
	 */
	ret = __is_suid(buf);
	if (ret == -1)
		return -1;
	else if (ret != 0)
		goto exec_sh;

	/* Do not proceed to any hack if not in chroot */
	if (!__inchroot()) {
		__note_if_not_preloading_libiamroot_and_ensure_preloading();
		__execfd();
		__verbose_exec(argv, __environ);
		return next_execve(path, argv, __environ);
	}

	ret = __interpreter_script(program, argv, buf, sizeof(buf), off,
				   interparg);
	if ((ret == -1) && (errno != ENOEXEC))
		return -1;
	if (ret < 1)
		goto loader;

	hashbang = &buf[off];
	for (i = 1; i < ret; i++)
		off += strnlen(&buf[off], sizeof(buf)-off)+1; /* NULL-terminated */

	/* FIXME: __interpreter_script() should do the following; it must have
	 * original and resolved path. */
	interparg[ret-1] = (char *)path; /* original program path as first
					  * positional argument */

	/*
	 * Preserve original path in argv0 and set the interpreter and its
	 * optional argument (if any).
	 */
	siz = path_resolution(AT_FDCWD, hashbang, &buf[off], sizeof(buf)-off,
			      0);
	if (siz == -1)
		return -1;
	program = &buf[off];
	off += siz+1; /* NULL-terminated */

loader:
	/* It is the dynamic loader */
	ret = __is_ldso(__basename(path));
	if (ret == -1)
		return -1;
	/* Try to run the dynamic loader internaly... */
	if (ret == 1) {
		int err;

		err = __ldso_execve(path, argv, envp);
		if (err == -1 && errno != EAGAIN)
			return -1;
	}
	/* ... or run it directly! */
	if (ret == 1) {
		__execfd();
		__verbose_exec(argv, envp);
		return next_execve(buf, argv, envp);
	}

	ret = __ldso(program, argv, interparg, buf, sizeof(buf), off);
	if ((ret == -1) && (errno != ENOEXEC))
		return -1;
	if (ret == -1)
		goto exec_sh;

	argc = 1;
	arg = interparg;
	while (*arg++)
		argc++;
	arg = argv+1; /* skip original-argv0 */
	while (*arg++)
		argc++;

	if ((argc > 0) && (argc < ARG_MAX)) {
		char *nargv[argc+1]; /* NULL-terminated */
		char **narg;

		narg = nargv;
		arg = interparg;
		while (*arg)
			*narg++ = *arg++;
		arg = argv+1; /* skip original-argv0 */
		while (*arg)
			*narg++ = *arg++;
		*narg++ = NULL; /* ensure NULL-terminated */

		__execfd();
		__verbose_exec(nargv, __environ);
		return next_execve(*nargv, nargv, __environ);
	}

	return __set_errno(E2BIG, -1);

exec_sh:
	ret = __exec_sh(path, argv, interparg, buf, sizeof(buf));
	if (ret == -1)
		return -1;

	argc = 1;
	arg = interparg;
	while (*arg++)
		argc++;
	arg = argv+1; /* skip original-argv0 */
	while (*arg++)
		argc++;

	if ((argc > 0) && (argc < ARG_MAX)) {
		char *nargv[argc+1]; /* NULL-terminated */
		char **narg;

		narg = nargv;
		arg = interparg;
		while (*arg)
			*narg++ = *arg++;
		arg = argv+1; /* skip original-argv0 */
		while (*arg)
			*narg++ = *arg++;
		*narg++ = NULL; /* ensure NULL-terminated */

		__execfd();
		__verbose_exec(nargv, __environ);
		return next_execve(*nargv, nargv, __environ);
	}

	return __set_errno(E2BIG, -1);
}
