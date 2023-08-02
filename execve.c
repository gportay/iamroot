/*
 * Copyright 2020-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <paths.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined __linux__ || defined __FreeBSD__
#include <sys/auxv.h>
#endif
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

extern int next_open(const char *, int, mode_t);
extern int next_fstatat(int, const char *, struct stat *, int);

#ifdef __linux__
static int __secure()
{
	return getauxval(AT_SECURE) != 0;
}

const char *__execfn()
{
	return (const char *)getauxval(AT_EXECFN);
}
#endif

#ifdef __FreeBSD__
const char *__execfn()
{
	static char buf[PATH_MAX];
	int ret;

	ret = elf_aux_info(AT_EXECPATH, buf, sizeof(buf));
	if (ret == -1)
		return NULL;

	return buf;
}
#endif

#ifdef __OpenBSD__
/*
 * Stolen and hacked from OpenBSD (usr.bin/top/machine.c)
 *
 * SPDX-FileCopyrightText: The OpenBSD Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <sys/sysctl.h>

#define err(e, r) return __set_errno((e), (r))

__attribute__((visibility("hidden")))
static char **get_proc_args()
{
	static char **s;
	static size_t siz = 1023;
	int mib[4];

	if (!s && !(s = malloc(siz)))
		err(1, NULL);

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC_ARGS;
	mib[2] = getpid();
	mib[3] = KERN_PROC_ARGV;
	for (;;) {
		size_t space = siz;
		if (sysctl(mib, 4, s, &space, NULL, 0) == 0)
			break;
		if (errno != ENOMEM)
			return NULL;
		siz *= 2;
		if ((s = realloc(s, siz)) == NULL)
			err(1, NULL);
	}
	return s;
}
#undef err

const char *__execfn()
{
	static char buf[PATH_MAX];
	char **argv;
	ssize_t siz;

	if (*buf)
		return buf;

	argv = get_proc_args();
	if (!argv)
		return NULL;

	if (*argv[0] == '/')
		siz = path_resolution(AT_FDCWD, argv[0], buf, sizeof(buf), 0);
	else
		siz = __path_access(argv[0], X_OK, getenv("PATH"), buf,
				    sizeof(buf));
	if (siz == -1)
		return NULL;

	return buf;
}
#endif

#ifdef __NetBSD__
extern ssize_t next_readlinkat(int, const char *, char *, size_t);

const char *__execfn()
{
	static char buf[PATH_MAX];
	ssize_t siz; 

	if (*buf)
		return buf;

	siz = next_readlinkat(AT_FDCWD, "/proc/self/exe", buf, sizeof(buf));
	fprintf(stderr, "%s:%i siz: %zi\n", __func__, __LINE__, siz);
	if (siz == -1)
		return NULL;
	buf[siz] = 0; /* ensure NULL-terminated */

	return buf;
}
#endif

__attribute__((visibility("hidden")))
const char *__path()
{
	const char *ret;

	ret = getenv("IAMROOT_PATH");
	if (!ret)
		ret = _PATH_STDPATH;

	return ret;
}

__attribute__((visibility("hidden")))
const char *__deflib()
{
	const char *ret;

	ret = getenv("IAMROOT_DEFLIB");
	if (!ret)
		ret = _PATH_DEFLIB;

	return ret;
}

static int __strtok(const char *str, const char *delim,
		    int (*callback)(const char *, void *), void *user)
{
	size_t len;

	if (!str || !callback)
		return __set_errno(EINVAL, -1);

	len = __strlen(str);
	if (len > 0) {
		char buf[len+1]; /* NULL-terminated */
		char *token, *saveptr;

		__strncpy(buf, str);
		token = strtok_r(buf, delim, &saveptr);
		do {
			int ret;

			if (!token)
				break;

			ret = callback(token, user);
			if (ret != 0)
				return ret;
		} while ((token = strtok_r(NULL, delim, &saveptr)));
	}

	return 0;
}

__attribute__((visibility("hidden")))
int __path_iterate(const char *path, int (*callback)(const char *, void *),
		 void *user)
{
	return __strtok(path, ":", callback, user);
}

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

	ignore = getenv("IAMROOT_EXEC_IGNORE");
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

__attribute__((visibility("hidden")))
int __issuid(const char *path)
{
	struct stat statbuf;
	int ret = -1;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, 0);
	if (ret == -1)
		return -1;

	return (statbuf.st_mode & S_ISUID) != 0;
}

static ssize_t __gethashbang(const char *path, char *buf, size_t bufsize)
{
	ssize_t ret;
	char *d, *s;
	int fd;

	/*
	 * According to man execve(2):
	 *
	 * Interpreter scripts
	 *
	 * An interpreter script is a text file that has execute permission
	 * enabled and whose first line is of the form:
	 *
	 * #!interpreter [optional-arg]
	 *
	 * The interpreter must be a valid pathname for an executable file.
	 *
	 * If the pathname argument of execve() specifies an interpreter
	 * script, then interpreter will be invoked with the following
	 * arguments:
	 *
	 * interpreter [optional-arg] pathname arg...
	 *
	 * where pathname is the pathname of the file specified as the first
	 * argument of execve(), and arg... is the series of words pointed to
	 * by the argv argument of execve(), starting at argv[1]. Note that
	 * there is no way to get the argv[0] that was passed to the execve()
	 * call.
	 *
	 * For portable use, optional-arg should either be absent, or be
	 * specified as a single word (i.e., it should not contain white
	 * space); see NOTES below.
	 *
	 * Since Linux 2.6.28, the kernel permits the interpreter of a script
	 * to itself be a script. This permission is recursive, up to a limit
	 * of four recursions, so that the interpreter may be a script which is
	 * interpreted by a script, and so on.
	 */
	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	ret = read(fd, buf, bufsize-1); /* NULL-terminated */
	if (ret == -1) {
		goto close;
	}
	buf[ret] = 0; /* ensure NULL-terminated */

	/* Not an hashbang interpreter directive */
	if ((ret < 2) || (buf[0] != '#') || (buf[1] != '!')) {
		ret = __set_errno(ENOEXEC, -1);
		goto close;
	}

	s = buf+2;
	d = buf;

	/* skip leading blanks */
	while (isblank(*s))
		s++;
	/* copy interpreter */
	while (*s && *s != '\n' && !isblank(*s))
		*d++ = *s++;
	*d++ = 0;

	/*
	 * According to man execve(2):
	 *
	 * Interpreter scripts
	 *
	 * The semantics of the optional-arg argument of an interpreter script
	 * vary across implementations. On Linux, the entire string following
	 * the interpreter name is passed as a single argument to the
	 * interpreter, and this string can include white space. However,
	 * behavior differs on some other systems. Some systems use the first
	 * white space to terminate optional-arg. On some systems, an
	 * interpreter script can have multiple arguments, and white spaces in
	 * optional-arg are used to delimit the arguments.
	 */
	/* has an optional argument */
	if (isblank(*s)) {
		/* skip leading blanks */
		while (isblank(*s))
			s++;
		/* copy optional argument */
		while (*s && *s != '\n' && !isblank(*s))
			*d++ = *s++;
		*d++ = 0;
	}

	ret = d-buf-1;

close:
	__close(fd);

	return ret;
}

#if !defined(NVERBOSE)
__attribute__((visibility("hidden")))
void __verbose_exec(const char *path, char * const argv[], char * const envp[])
{
	int color, fd, debug;

	debug = __getdebug();
	if (debug == 0)
		return;

	fd = __getdebug_fd();
	if (fd < 0)
		return;

	color = __getcolor();
	if (color)
		dprintf(fd, "\033[32;1m");

	dprintf(fd, "Debug: ");

	if (color)
		dprintf(fd, "\033[0m");

	if (debug < 4) {
		char * const *p;

		dprintf(fd, "running");

		p = envp;
		while (*p)
			dprintf(fd, " \"%s\"", *p++);

		p = argv;
		while (*p)
			dprintf(fd, " \"%s\"", *p++);
		dprintf(fd, "\n");
	} else {
		char * const *p;

		dprintf(fd, "execve(path: '%s', argv: {", path);

		p = argv;
		while (*p)
			dprintf(fd, " '%s',", *p++);
		dprintf(fd, " NULL }, ");
		if (debug < 5) {
			dprintf(fd, "envp: %p)\n", envp);
		} else {
			dprintf(fd, "envp: %p {", envp);
			p = envp;
			while (*p)
				dprintf(fd, " %s", *p++);
			dprintf(fd, " }\n");
		}
	}
}
#endif

__attribute__((visibility("hidden")))
const char *__getexe()
{
	const char *exec;
	size_t len;
	char *root;

	exec = __execfn();
	if (!exec)
		return NULL;

	len = 0;
	root = __getroot();
	if (root)
		len = __strlen(root);

	if (__strlen(exec) < len)
		return NULL;

	return &exec[len];
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

static char *__getexec()
{
	char *ret;

	ret = getenv("IAMROOT_EXEC");
#ifdef __linux__
	if (!ret)
		return "/usr/lib/iamroot/exec.sh";
#else
	if (!ret)
		return "/usr/local/lib/iamroot/exec.sh";
#endif

	return ret;
}

__attribute__((visibility("hidden")))
int __execve(const char *path, char * const argv[], char * const envp[])
{
	const char *root;
	ssize_t len;

	root = __getrootdir();
	if (streq(root, "/"))
		goto exit;

	len = __strlen(root);
	if (strneq(path, root, len))
		path += len;

exit:
	return execve(path, argv, envp);
}

__attribute__((visibility("hidden")))
int __hashbang(const char *path, char * const argv[], char *interp,
	       size_t interpsiz, char *interparg[])
{
	ssize_t siz;
	size_t len;
	int i = 0;
	(void)argv;

	/* Get the interpeter directive stored after the hashbang */
	siz = __gethashbang(path, interp, interpsiz);
	if (siz < 1)
		return siz;

	/* Reset argv0 */
	interparg[i++] = interp; /* hashbang as argv0 */

	/* Add optional argument */
	len = strnlen(interp, interpsiz);
	if (len < (size_t)siz && interp[len+1])
		interparg[i++] = &interp[len+1];
	interparg[i++] = (char *)path; /* FIXME: original program path as first
					* positional argument */
	interparg[i] = NULL; /* ensure NULL-terminated */

	return i;
}

__attribute__((visibility("hidden")))
int __exec_sh(const char *path, char * const *argv, char *interparg[],
	      char *buf, size_t bufsize)
{
	int ret, i;

	i = 0;
	interparg[i++] = _strncpy(buf, __getexec(), bufsize);
	interparg[i++] = (char *)path; /* original path as first positional
					* argument
					*/
	interparg[i] = NULL; /* ensure NULL-terminated */

	ret = setenv("argv0", *argv, 1);
	if (ret)
		return -1;

	ret = setenv("ld_preload", getenv("LD_PRELOAD") ?: "", 1);
	if (ret)
		return -1;

	ret = setenv("ld_library_path", getenv("LD_LIBRARY_PATH") ?: "", 1);
	if (ret)
		return -1;

	ret = unsetenv("LD_PRELOAD");
	if (ret)
		return -1;

	ret = unsetenv("LD_LIBRARY_PATH");
	if (ret)
		return -1;

	return i;
}

int execve(const char *path, char * const argv[], char * const envp[])
{
	char *interparg[14+1] = { NULL }; /*  0 ARGV0
					   *  1 /lib/ld.so
					   *  2 LD_LINUX_ARGV1
					   *  3 --preload
					   *  4 libiamroot.so:libc.so:libdl.so
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
	char hashbang[HASHBANG_MAX];
	char hashbangbuf[PATH_MAX];
	char loaderbuf[PATH_MAX];
	char *program = NULL;
	char buf[PATH_MAX];
	char * const *arg;
	int argc, ret;
	ssize_t siz;

	/* Run exec.sh script */
	if (__exec_ignored(path))
		goto exec_sh;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return -1;

	__debug("%s(path: '%s' -> '%s', argv: { '%s', '%s', ... }, envp: %p)\n",
		__func__, path, buf, argv[0], argv[1], envp);

	interparg[0] = *argv; /* original argv0 as argv0 */
	program = buf;

	/*
	 * In secure-execution mode, preload pathnames containing slashes are
	 * ignored. Furthermore, shared objects are preloaded only from the
	 * standard search directories and only if they have set-user-ID mode
	 * bit enabled (which is not typical).
	 */
	ret = __issuid(buf);
	if (ret == -1)
		return -1;
	else if (ret != 0)
		goto exec_sh;

	/* Do not proceed to any hack if not in chroot */
	if (!__inchroot()) {
		__execfd();
		__verbose_exec(path, argv, envp);
		return next_execve(path, argv, envp);
	}

	ret = __hashbang(program, argv, hashbang, sizeof(hashbang), interparg);
	if ((ret == -1) && (errno != ENOEXEC))
		return -1;
	if (ret < 1)
		goto loader;

	/* FIXME: __hashbang() should do the following; it must have original
	 * and resolved path. */
	interparg[ret-1] = (char *)path; /* original program path as first
					  * positional argument */

	/*
	 * Preserve original path in argv0 and set the interpreter and its
	 * optional argument (if any).
	 */
	siz = path_resolution(AT_FDCWD, hashbang, hashbangbuf,
			      sizeof(hashbangbuf), 0);
	if (siz == -1)
		return -1;

	program = hashbangbuf;

loader:
	/* It is the dynamic loader */
	ret = __is_ldso(__basename(path));
	if (ret == -1)
		return -1;
	/* Try to run the dynamic loader internaly... */
	if (ret == 1) {
		int err;

		err = __ldso_execv(path, argv, envp);
		if (err == -1 && errno != EAGAIN)
			return -1;
	}
	/* ... or run it directly! */
	if (ret == 1) {
		__execfd();
		__verbose_exec(buf, argv, envp);
		return next_execve(buf, argv, envp);
	}

	ret = __loader(program, argv, loaderbuf, sizeof(loaderbuf), interparg);
	if ((ret == -1) && (errno != ENOEXEC))
		return -1;
	if (ret != -1)
		goto execve;

exec_sh:
	ret = __exec_sh(path, argv, interparg, buf, sizeof(buf));
	if (ret == -1)
		return -1;

execve:
	ret = setenv("IAMROOT_VERSION", __xstr(VERSION), 1);
	if (ret)
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
		__verbose_exec(*nargv, nargv, __environ);
		return next_execve(*nargv, nargv, __environ);
	}

	return __set_errno(EINVAL, -1);
}
