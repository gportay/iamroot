/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>
#ifdef __linux__
#include <linux/limits.h>
#endif

#include <sys/types.h>
#include <sys/xattr.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_lremovexattr(const char *path, const char *name)
{
	int (*sym)(const char *, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "lremovexattr");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, name);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int lremovexattr(const char *path, const char *name)
{
	char xbuf[XATTR_NAME_MAX + 1];
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s', name: '%s', ...)\n", __func__, path,
		  real_path, name);

	if (__strncmp(name, "user") != 0) {
		int ret;

		ret = _snprintf(xbuf, sizeof(xbuf), "%s.%s",
				"user.iamroot", name);
		if (ret == -1) {
			errno = ERANGE;
			return -1;
		}

		name = xbuf;
	}

	return next_lremovexattr(real_path, name);
}
