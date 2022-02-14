/*
 * Copyright 2021-2022 Gaël PORTAY
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
ssize_t next_getxattr(const char *path, const char *name, void *value,
		      size_t size)
{
	ssize_t (*sym)(const char *, const char *, void *, size_t);
	ssize_t ret;

	sym = dlsym(RTLD_NEXT, "getxattr");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, name, value, size);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
	char xbuf[XATTR_NAME_MAX + 1];
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', name: '%s', ...)\n", __func__, path,
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

	return next_getxattr(real_path, name, value, size);
}
