/*
 * Copyright 2022 Gaël PORTAY
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

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next___utimensat64(int fd, const char *path,
		       const struct timespec times[2], int flags)
{
	int (*sym)(int, const char *, const struct timespec[2], int);
	int ret;

	sym = dlsym(RTLD_NEXT, "__utimensat64");
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

int __utimensat64(int fd, const char *path, const struct timespec times[2],
		  int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(fd, path, buf, sizeof(buf), AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', flags: 0x%x)\n", __func__, fd,
		path, buf, flags);

	return next___utimensat64(fd, buf, times, flags);
}
#endif
