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
ssize_t next_fgetxattr(int fd, const char *name, void *value, size_t size)
{
	ssize_t (*sym)(int, const char *, void *, size_t);
	ssize_t ret;

	sym = dlsym(RTLD_NEXT, "fgetxattr");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, name, value, size);
	if (ret == -1)
		__fperror(fd, __func__);

	return ret;
}

ssize_t fgetxattr(int fd, const char *name, void *value, size_t size)
{
	char xbuf[XATTR_NAME_MAX + 1];

	__verbose_func("%s(fd: %i, name: '%s', ...)\n", __func__, fd, name);

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

	return next_fgetxattr(fd, name, value, size);
}
