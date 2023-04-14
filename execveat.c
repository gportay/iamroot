/*
 * Copyright 2022-2023 Gaël PORTAY
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

__attribute__((visibility("hidden")))
int next_execveat(int dfd, const char *path, char * const argv[],
		  char * const envp[], int atflags)
{
	int (*sym)(int, const char *, char * const[], char * const[],
		   int);
	int ret;

	sym = dlsym(RTLD_NEXT, "execveat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(dfd, path, argv, envp, atflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int execveat(int dfd, const char *path, char * const argv[],
	     char * const envp[], int atflags)
{
	char *interparg[15+1] = { NULL }; /*  0 ARGV0
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
					   * 13 HASHBANG_ARGV1
					   * 14 -x
					   * 15 script.sh
					   * 16 NULL-terminated
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

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', argv: { '%s', '%s', ... }, envp: %p, atflags: 0x%x)\n",
		__func__, dfd, __fpath(dfd), path, buf, argv[0], argv[1], envp,
		atflags);

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
	else if (ret)
		goto exec_sh;

	/* Do not proceed to any hack if not in chroot */
	if (!__inchroot()) {
		__verbose_exec(path, argv, envp);
		return next_execveat(dfd, path, argv, envp, atflags);
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
	/*
	 * Run the dynamic linker directly
	 */
	if ((__strncmp(path, "/usr/bin/ld.so") == 0) ||
	    (__strncmp(path, "/lib/ld") == 0) ||
	    (__strncmp(path, "/lib64/ld") == 0)) {
		__verbose_exec(buf, argv, envp);
		return next_execveat(dfd, buf, argv, envp, atflags);
	}

	ret = __loader(program, argv, loaderbuf, sizeof(loaderbuf), interparg);
	if ((ret == -1) && (errno != ENOEXEC))
		return -1;
	if (ret != -1)
		goto execveat;

exec_sh:
	ret = __exec_sh(path, argv, interparg, buf, sizeof(buf));
	if (ret == -1)
		return -1;

execveat:
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

		__verbose_exec(*nargv, nargv, __environ);
		__remove_at_empty_path_if_needed(*nargv, atflags);
		return next_execveat(dfd, *nargv, nargv, __environ, atflags);
	}

	errno = EINVAL;
	return -1;
}
