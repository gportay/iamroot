/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/statfs.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_statfs(const char *path, struct statfs *statfsbuf)
{
	int (*sym)(const char *, struct statfs *);
	int ret;

	sym = dlsym(RTLD_NEXT, "statfs");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, statfsbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int statfs(const char *path, struct statfs *statfsbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return next_statfs(real_path, statfsbuf);
}
