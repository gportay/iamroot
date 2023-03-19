/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/stat.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif
#ifdef __FreeBSD__
#include <sys/extattr.h>
#endif

#include <fcntl.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_openat(int dfd, const char *path, int oflags, mode_t mode)
{
	int (*sym)(int, const char *, int, ...);
	int ret;

	sym = dlsym(RTLD_NEXT, "openat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(dfd, path, oflags, mode);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int openat(int dfd, const char *path, int oflags, ...)
{
	mode_t oldmode = 0, mode = 0;
	char buf[PATH_MAX];
	int atflags = 0;
	ssize_t siz;
	int ret;

	if (oflags & O_NOFOLLOW)
		atflags = AT_SYMLINK_NOFOLLOW;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

#ifdef __linux__
	if ((oflags & O_CREAT) || (oflags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, oflags);
		mode = va_arg(ap, mode_t);
		oldmode = mode;
		va_end(ap);
	}
#endif

	if (oflags & O_CREAT)
		__fwarn_if_insuffisant_user_modeat(dfd, buf, mode, 0);
	__debug("%s(dfd: %i, path: '%s' -> '%s', oflags: 0%o, mode: 0%03o -> 0%03o)\n",
		__func__, dfd, path, buf, oflags, oldmode, mode);

	ret = next_openat(dfd, buf, oflags, mode);
	__set_mode(buf, oldmode, mode);

	return ret;
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(openat, openat64);
#endif

#ifdef __GLIBC__
weak_alias(openat, __openat_2);
#ifdef _LARGEFILE64_SOURCE
weak_alias(openat, __openat64_2);
#endif
#endif
