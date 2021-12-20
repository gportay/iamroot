/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/statvfs.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next_statvfs64(const char *path, struct statvfs64 *statvfsbuf)
{
	int (*sym)(const char *, struct statvfs64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "statvfs64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, statvfsbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int statvfs64(const char *path, struct statvfs64 *statvfsbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return next_statvfs64(real_path, statvfsbuf);
}
#endif
