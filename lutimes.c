/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/time.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_lutimes(const char *path, const struct timeval times[2])
{
	int (*sym)(const char *, const struct timeval[2]);
	int ret;

	sym = dlsym(RTLD_NEXT, "lutimes");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, times);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int lutimes(const char *path, const struct timeval times[2])
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_lutimes(real_path, times);
}
