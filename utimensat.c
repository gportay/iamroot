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
	ssize_t siz;

	siz = path_resolution(fd, path, buf, sizeof(buf), AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', flags: 0x%x)\n", __func__, fd,
		path, buf, flags);

	__remove_at_empty_path_if_needed(buf, flags);
	return next_utimensat(fd, buf, times, flags);
}

#ifdef __GLIBC__
weak_alias(utimensat, __utimensat64);
#endif
