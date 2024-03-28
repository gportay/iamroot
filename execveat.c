/*
 * Copyright 2022-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

extern int __ldso_execveat(int, const char *, char * const[], char * const[]);

static int (*sym)(int, const char *, char * const[], char * const[], int);

hidden int next_execveat(int dfd, const char *path, char * const argv[],
			 char * const envp[], int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "execveat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, argv, envp, atflags);
}

int execveat(int dfd, const char *path, char * const argv[],
	     char * const envp[], int atflags)
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

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return -1;
	program = buf;
	off += siz+1; /* NULL-terminated */

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', argv: { '%s', '%s', ... }, envp: %p, atflags: 0x%x)\n",
		__func__, dfd, __fpath(dfd), path, buf, argv[0], argv[1], envp,
		atflags);

	interparg[0] = *argv; /* original argv0 as argv0 */

	ret = __can_exec(program);
	if (ret == -1)
		return -1;
	else if (ret == 0)
		goto exec_sh;

	/* Do not proceed to any hack if not in chroot */
	if (!__inchroot()) {
		__note_if_not_preloading_libiamroot_and_ensure_preloading();
		__execfd();
		__verbose_exec(argv, __environ);
		return next_execveat(dfd, path, argv, __environ, atflags);
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
	/* Try to run the dynamic loader internaly... */
	if (ret == 1) {
		int err;

		err = __ldso_execveat(dfd, path, argv, envp);
		if (err == -1 && errno != EAGAIN)
			return -1;
	}
	/* ... or run it directly! */
	if (ret == 1) {
		__execfd();
		__verbose_exec(argv, envp);
		return next_execveat(dfd, buf, argv, envp, atflags);
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
		return next_execveat(dfd, *nargv, nargv, __environ, atflags);
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
		return next_execveat(dfd, *nargv, nargv, __environ, atflags);
	}

	return __set_errno(E2BIG, -1);
}
