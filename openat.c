/*
 * Copyright 2021 Gaël PORTAY
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
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, flags, mode);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int openat(int fd, const char *path, int flags, ...)
{
	char buf[PATH_MAX];
	char *real_path;
	mode_t mode = 0;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	__verbose_func("%s(path: '%s' -> '%s', flags: %x, mode: 0%03o)\n",
		       __func__, path, real_path, flags, mode);
	if (flags & O_CREAT)
		__fwarn_if_insuffisant_user_modeat(fd, real_path, mode, 0);

	return next_openat(fd, real_path, flags, mode);
}
