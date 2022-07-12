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
int next_openat(int fd, const char *path, int flags, mode_t mode)
{
	int (*sym)(int, const char *, int, ...);
	int ret;

	sym = dlsym(RTLD_NEXT, "openat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, flags, mode);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int openat(int fd, const char *path, int flags, ...)
{
	char buf[PATH_MAX];
	int atflags = 0;
	mode_t mode = 0;
	ssize_t siz;

	if (flags & O_NOFOLLOW)
		atflags = AT_SYMLINK_NOFOLLOW;

	siz = path_resolution(fd, path, buf, sizeof(buf), atflags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

#ifdef __linux__
	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
#endif

	__debug("%s(fd: %i, path: '%s' -> '%s', flags: 0%o, mode: 0%03o)\n",
		__func__, fd, path, buf, flags, mode);
	if (flags & O_CREAT)
		__fwarn_if_insuffisant_user_modeat(fd, buf, mode, 0);

	return next_openat(fd, buf, flags, mode);
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(openat, openat64);
#endif
