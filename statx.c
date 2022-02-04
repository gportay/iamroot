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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "iamroot.h"

#ifdef __GLIBC__
extern int rootstatx(int, const char *, int, unsigned int, struct statx *);

__attribute__((visibility("hidden")))
int next_statx(int fd, const char *path, int flags, unsigned int mask,
	       struct statx *statxbuf)
{
	int (*sym)(int, const char *, int, unsigned int, struct statx *);
	int ret;

	sym = dlsym(RTLD_NEXT, "statx");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, flags, mask, statxbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int statx(int fd, const char *path, int flags, unsigned int mask,
	  struct statx *statxbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', flags: 0x%x...)\n", __func__,
		fd, path, real_path, flags);

	return rootstatx(fd, real_path, flags, mask, statxbuf);
}
#endif
