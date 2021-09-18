/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
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
		__perror(path, __func__);

	return ret;
}

int open(const char *path, int flags, ...)
{
	char buf[PATH_MAX];
	char *real_path;
	mode_t mode = 0;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	__verbose("%s(path: '%s' -> '%s', flags: %x, mode: %o)\n", __func__,
		  path, real_path, flags, mode);

	return next_open(real_path, flags, mode);
}
