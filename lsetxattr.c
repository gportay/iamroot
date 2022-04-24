/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <linux/limits.h>

#include <sys/types.h>
#include <sys/xattr.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_lsetxattr(const char *path, const char *name, const void *value,
		   size_t size, int flags)
{
	int (*sym)(const char *, const char *, const void *, size_t, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "lsetxattr");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, name, value, size, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int lsetxattr(const char *path, const char *name, const void *value,
	      size_t size, int flags)
{
	char xbuf[XATTR_NAME_MAX + 1];
	char buf[PATH_MAX];

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			    AT_SYMLINK_NOFOLLOW) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', name: '%s', ...)\n", __func__, path,
		buf, name);

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

	return next_lsetxattr(buf, name, value, size, flags);
}
#endif
