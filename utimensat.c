/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_utimensat(int fd, const char *path, const struct timespec times[2],
		   int flags)
{
	int (*sym)(int, const char *, const struct timespec[2], int);
	int ret;

	sym = dlsym(RTLD_NEXT, "utimensat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, times, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int utimensat(int fd, const char *path, const struct timespec times[2],
	      int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(fd, path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', flags: 0x%x)\n", __func__, fd,
		path, real_path, flags);

	return next_utimensat(fd, real_path, times, flags);
}
