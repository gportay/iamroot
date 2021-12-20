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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_open(const char *path, int flags, mode_t mode)
{
	int (*sym)(const char *, int, ...);
	int ret;

	sym = dlsym(RTLD_NEXT, "open");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, flags, mode);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int open(const char *path, int flags, ...)
{
	char buf[PATH_MAX];
	char *real_path;
	mode_t mode = 0;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	__debug("%s(path: '%s' -> '%s', flags: %x, mode: 0%03o)\n", __func__,
		path, real_path, flags, mode);
	if (flags & O_CREAT)
		__warn_if_insuffisant_user_mode(real_path, mode);

	return next_open(real_path, flags, mode);
}
