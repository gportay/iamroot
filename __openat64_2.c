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

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next_openat64_2(int fd, const char *path, int flags)
{
	int (*sym)(int, const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "openat64_2");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int openat64_2(int fd, const char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;
	int atflags = 0;

	if (flags & O_NOFOLLOW)
		atflags = AT_SYMLINK_NOFOLLOW;

	real_path = path_resolution(fd, path, buf, sizeof(buf), atflags);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(fd: %d, path: '%s' -> '%s', flags: 0%o)\n", __func__, fd,
		path, real_path, flags);

	return next_openat64_2(fd, real_path, flags);
}
#endif
