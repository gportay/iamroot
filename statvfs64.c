/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
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
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path, statvfsbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int statvfs64(const char *path, struct statvfs64 *statvfsbuf)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return next_statvfs64(buf, statvfsbuf);
}
#endif
