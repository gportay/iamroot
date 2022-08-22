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
	char buf[PATH_MAX];
	int atflags = 0;
	mode_t mode = 0;
	ssize_t siz;

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
		va_end(ap);
	}
#endif

	__debug("%s(dfd: %i, path: '%s' -> '%s', oflags: 0%o, mode: 0%03o)\n",
		__func__, dfd, path, buf, oflags, mode);
	if (oflags & O_CREAT)
		__fwarn_if_insuffisant_user_modeat(dfd, buf, mode, 0);

	return next_openat(dfd, buf, oflags, mode);
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(openat, openat64);
#endif
