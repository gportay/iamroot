/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <dlfcn.h>
#ifdef __linux__
#include <linux/limits.h>
#endif

#include <sys/types.h>
#include <sys/xattr.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fsetxattr(int fd, const char *name, const void *value, size_t size,
		   int flags)
{
	int (*sym)(int, const char *, const void *, size_t, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fsetxattr");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, name, value, size, flags);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fsetxattr(int fd, const char *name, const void *value, size_t size,
	      int flags)
{
	char xbuf[XATTR_NAME_MAX + 1];

	__debug("%s(fd: %i, name: '%s', ...)\n", __func__, fd, name);

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

	return next_fsetxattr(fd, name, value, size, flags);
}
