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
#include <dlfcn.h>
#include <linux/limits.h>

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
	char xbuf[XATTR_NAME_MAX+1]; /* NULL-terminated */
	char buf[PATH_MAX];
	ssize_t siz;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, "__procfdreadlink");
		return -1;
	}
	buf[siz] = 0;

	if (__strncmp(name, "user.iamroot.") != 0) {
		int ret;

		ret = _snprintf(xbuf, sizeof(xbuf), "%s.%s",
				"user.iamroot", name);
		if (ret == -1) {
			errno = ERANGE;
			return -1;
		}

		name = xbuf;
	}

	__debug("%s(fd: %i <-> '%s', name: '%s', ..., flags: 0x%x)\n",
		__func__, fd, buf, name, flags);

	return next_fsetxattr(fd, name, value, size, flags);
}
#endif
