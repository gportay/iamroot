/*
 * Copyright 2022 Gaël PORTAY
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
int next_execveat(int fd, const char *path, char * const argv[],
		  char * const envp[], int flags)
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

	ret = sym(fd, path, argv, envp, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

static int __loader(const char *path, char * const argv[], char *interp,
		    size_t interpsiz, char *interparg[])
{
	char buf[HASHBANG_MAX];
	ssize_t siz;

	/*
	 * Get the dynamic linker stored in the .interp section of the ELF
	 * linked program.
	 */
	siz = getinterp(path, buf, sizeof(buf));
	if (siz < 1)
		return siz;

	/*
	 * The interpreter has to preload its libiamroot.so library.
	 */
	if ((__strncmp(buf, "/lib/ld") == 0) ||
	    (__strncmp(buf, "/lib64/ld") == 0)) {
		char *argv0, *xargv1, *rpath, *runpath, *ld_preload,
		     *ld_library_path, *inhibit_rpath;
		int has_argv0 = 1, has_preload = 1, has_inhibit_rpath = 0,
		    has_inhibit_cache = 0;
		int ret, i, j, shift = 1, abi = 0;
		const char *basename;
		char ldso[NAME_MAX];
		char * const *arg;

		basename = __basename(buf);
		ret = sscanf(basename, "ld-%[^.].so.%d", ldso, &abi);
		if (ret < 2) {
			errno = ENOTSUP;
			return -1;
		}

		/*
		 * the glibc world supports --argv0 since 2.33, --preload since
		 * 2.30, --inhibit-cache since 2.16, and --inhibit-rpath since
		 * 2.0.94
		 */
		if (__strncmp(ldso, "linux") == 0) {
			has_inhibit_rpath = 1;

			has_inhibit_cache =
				      __ld_linux_has_inhibit_cache_option(buf);
			if (has_inhibit_cache == -1)
				return -1;

			has_argv0 = __ld_linux_has_argv0_option(buf);
			if (has_argv0 == -1)
				return -1;

			has_preload = __ld_linux_has_preload_option(buf);
			if (has_preload == -1)
				return -1;
		}

		rpath = __rpath(path);
		if (rpath)
			__info("%s: RPATH=%s\n", path, rpath);

		runpath = __runpath(path);
		if (runpath)
			__info("%s: RUNPATH=%s\n", path, runpath);

		ld_preload = __ld_preload(ldso, abi);
		if (!ld_preload)
			__warning("%s: is unset!\n", "ld_preload");

		ld_library_path = __ld_library_path(ldso, abi);
		if (!ld_library_path)
			__warning("%s: is unset!\n", "ld_library_path");

		inhibit_rpath = __inhibit_rpath();
		if (!inhibit_rpath)
			__notice("%s: is unset!\n", "inhibit_rpath");

		siz = path_resolution(AT_FDCWD, buf, interp, interpsiz, 0);
		if (siz == -1)
			return -1;

		/*
		 * Shift enough room in interparg to prepend:
		 *   - the path to the interpreter (i.e. the absolute path in
		 *     host, including the chroot; argv0).
		 *   - the optional extra argument as argv1.
		 *   - the option --ld-preload and its argument (i.e. the path
		 *     in host environment to the interpreter's libiamroot.so,
		 *     and the path in chroot environment to the interpreter's
		 *     libc.so and libdl.so).
		 *   - the option --library-path and its argument (i.e. the
		 *     path in chroot environment to the libraries).
		 *   - the option --inhibit-rpath and its argument (i.e. the
		 *     path in host environment to the libbraries).
		 *   - the option --inhibit-cache.
		 *   - the option --argv0 and its argument (i.e. the original
		 *     path in host to the binary).
		 *   - the path to the binary (i.e. the full path in chroot,
		 *     *not* including chroot; first positional argument).
		 * Note: the binary's arguments are the original argv shifted
		 *       by one (i.e. without argv0; following arguments).
		 */
		argv0 = interparg[0];
		xargv1 = getenv("IAMROOT_EXEC_LD_ARGV1");
		if (xargv1)
			shift++;
		if (has_argv0)
			shift += 2;
		if (has_preload && ld_preload)
			shift += 2;
		if (ld_library_path)
			shift += 2;
		if (has_inhibit_rpath && inhibit_rpath)
			shift += 2;
		if (has_inhibit_cache)
			shift++;
		i = 0;
		for (arg = interparg; *arg; arg++)
			i++;
		for (j = 0; j < i; j++)
			interparg[j+shift] = interparg[j];

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = interp;

		/* Add extra argument as argv1 */
		if (xargv1)
			interparg[i++] = xargv1;

		/*
		 * Add --preload and interpreter's libraries:
		 *  - libiamroot.so (from host)
		 *  - libc.so and libdl.so (from chroot)
		 */
		if (has_preload && ld_preload) {
			interparg[i++] = "--preload";
			interparg[i++] = ld_preload;

			ret = unsetenv("LD_PRELOAD");
			if (ret) {
				__envperror("LD_PRELOAD", "unsetenv");
				return -1;
			}
		} else {
			ret = setenv("LD_PRELOAD", ld_preload, 1);
			if (ret)
				return -1;
		}

		/* Add --library-path (chroot) */
		if (ld_library_path) {
			interparg[i++] = "--library-path";
			interparg[i++] = ld_library_path;
		}

		/* Add --inhibit-rpath (chroot) */
		if (has_inhibit_rpath && inhibit_rpath) {
			interparg[i++] = "--inhibit-rpath";
			interparg[i++] = inhibit_rpath;
		}

		/* Add --inhibit-cache */
		if (has_inhibit_cache)
			interparg[i++] = "--inhibit-cache";

		/* Add --argv0 and original argv0 */
		if (has_argv0) {
			interparg[i++] = "--argv0";
			interparg[i++] = argv0;
		} else {
			/*
			 * The dynamic loader does not support for the option
			 * --argv0; the value will be set by via the function
			 * __libc_start_main().
			 */
			ret = setenv("argv0", argv0, 1);
			if (ret)
				return -1;
		}

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i] = (char *)path;
		i += j;
		interparg[i] = NULL; /* ensure NULL terminated */

		return i;
	} else {
		int i, j, shift = 1;
		char * const *arg;
		char *xargv1;

		siz = path_resolution(AT_FDCWD, buf, interp, interpsiz, 0);
		if (siz == -1)
			return -1;

		/*
		 * Shift enough room in interparg to prepend:
		 *   - the path to the interpreter (i.e. the absolute path in
		 *     host, including the chroot; argv0).
		 *   - the optional extra argument as argv1.
		 *   - the path to the binary (i.e. the full path in chroot,
		 *     *not* including chroot; first positional argument).
		 * Note: the binary's arguments are the original argv shifted
		 *       by one (i.e. without argv0; following arguments).
		 */
		xargv1 = getenv("IAMROOT_EXEC_LD_ARGV1");
		if (xargv1)
			shift++;
		i = 0;
		for (arg = argv; *arg; arg++)
			i++;
		for (j = 0; j < i; j++)
			interparg[j+shift] = interparg[j];

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = interp;

		/* Add extra argument as argv1 */
		if (xargv1)
			interparg[i++] = xargv1;

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i] = (char *)path;
		i += j;
		interparg[i] = NULL; /* ensure NULL terminated */

		return i;
	}

	return 0;
}

static int __exec_sh(const char *path, char * const *argv, char **interparg,
		     char *buf, size_t bufsize)
{
	int ret, i;

	i = 0;
	interparg[i++] = _strncpy(buf, __getexec(), bufsize);
	interparg[i++] = (char *)path; /* original path as first positional
					* argument
					*/
	interparg[i] = NULL; /* ensure NULL terminated */

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

int execveat(int fd, const char *path, char * const argv[],
	     char * const envp[], int flags)
{
	char buf[PATH_MAX], hashbangbuf[PATH_MAX], hashbang[HASHBANG_MAX];
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
					   * 16 NULL
					   */
	char loaderbuf[PATH_MAX];
	ssize_t hashbangsiz, siz;
	char *program = NULL;
	char * const *arg;
	int i, ret, argc;
	char *xargv1;
	size_t len;

	/* Run exec.sh script */
	if (exec_ignored(path))
		goto exec_sh;

	siz = path_resolution(fd, path, buf, sizeof(buf), flags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd, %d, path: '%s' -> '%s', argv: { '%s', '%s', ... }, envp: %p, flags: 0x%x)\n",
		__func__, fd, path, buf, argv[0], argv[1], envp, flags);
	i = 0;
	interparg[i++] = *argv; /* original argv0 as argv0 */
	program = buf;

	/*
	 * In secure-execution mode, preload pathnames containing slashes are
	 * ignored. Furthermore, shared objects are preloaded only from the
	 * standard search directories and only if they have set-user-ID mode
	 * bit enabled (which is not typical).
	 */
	ret = issuid(buf);
	if (ret == -1)
		return -1;
	else if (ret)
		goto exec_sh;

	/* Do not proceed to any hack if not in chroot */
	if (!inchroot())
		return next_execveat(fd, path, argv, envp, flags);

	/* Get the interpeter directive stored after the hashbang */
	hashbangsiz = gethashbang(program, hashbang, sizeof(hashbang));
	if (hashbangsiz == -1) {
		/* Not an hashbang interpreter directive */
		if (errno == ENOEXEC)
			goto loader;

		return -1;
	} else if (hashbangsiz == 0) {
		goto loader;
	}

	/*
	 * Preserve original path in argv0 and set the interpreter and its
	 * optional argument (if any).
	 */
	siz = path_resolution(AT_FDCWD, hashbang, hashbangbuf,
			      sizeof(hashbangbuf), 0);
	if (siz == -1)
		return -1;

	/* Reset argv0 */
	interparg[0] = hashbang; /* hashbang as argv0 */

	/* Add extra argument */
	xargv1 = getenv("IAMROOT_EXEC_HASHBANG_ARGV1");
	if (xargv1)
		interparg[i++] = xargv1; /* extra argument as argv1 */
	/* Add optional argument */
	len = __strnlen(hashbang);
	if (len < (size_t)hashbangsiz && hashbang[len+1])
		interparg[i++] = &hashbang[len+1];
	interparg[i++] = (char *)path; /* original program path as first
					* positional argument */
	interparg[i] = NULL; /* ensure NULL terminated */
	program = hashbangbuf;

loader:
	/*
	 * Run the dynamic linker directly
	 */
	if ((__strncmp(path, "/usr/bin/ld.so") == 0) ||
	    (__strncmp(path, "/lib/ld") == 0) ||
	    (__strncmp(path, "/lib64/ld") == 0)) {
		verbose_exec(buf, argv, envp);
		return next_execveat(fd, buf, argv, envp, flags);
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

	program = interparg[0];

execveat:
	ret = setenv("IAMROOT_VERSION", __xstr(VERSION), 1);
	if (ret)
		return -1;

	argc = 1;
	arg = interparg;
	while (*arg++)
		argc++;
	arg = argv+1;
	while (*arg++)
		argc++;

	if ((argc > 0) && (argc < ARG_MAX)) {
		char *nargv[argc+1]; /* NULL */
		char **narg;

		narg = nargv;
		arg = interparg;
		while (*arg)
			*narg++ = *arg++;
		arg = argv+1;
		while (*arg)
			*narg++ = *arg++;
		*narg++ = NULL;

		verbose_exec(*nargv, nargv, __environ);
		return next_execveat(fd, *nargv, nargv, __environ, flags);
	}

	errno = EINVAL;
	return -1;
}
