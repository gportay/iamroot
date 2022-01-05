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

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_mkdirat(int fd, const char *path, mode_t mode)
{
	int (*sym)(int, const char *, mode_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "mkdirat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, mode);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int mkdirat(int fd, const char *path, mode_t mode)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(fd: %d, path: '%s' -> '%s', mode: 0%03o)\n", __func__, fd,
		path, real_path, mode);
	__fwarn_if_insuffisant_user_modeat(fd, real_path, mode, 0);

	return next_mkdirat(fd, real_path, mode);
}
