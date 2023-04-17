/*
 * Copyright 2021-2023 Gaël PORTAY
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
int next_removexattr(const char *path, const char *name)
{
	int (*sym)(const char *, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "removexattr");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path, name);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int removexattr(const char *path, const char *name)
{
	char xbuf[XATTR_NAME_MAX+1]; /* NULL-terminated */
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	if (__strncmp(name, IAMROOT_XATTRS_PREFIX) != 0) {
		int ret;

		ret = _snprintf(xbuf, sizeof(xbuf), "%s%s",
				IAMROOT_XATTRS_PREFIX, name);
		if (ret == -1)
			return -1;

		name = xbuf;
	}

	__debug("%s(path: '%s' -> '%s', name: '%s' -> '%s', ...)\n", __func__,
		path, buf, name, xbuf);

	return next_removexattr(buf, name);
}
#endif
