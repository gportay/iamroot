/*
 * Copyright 2022 Gaël PORTAY
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
int next_fremovexattr(int fd, const char *name)
{
	int (*sym)(int, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fremovexattr");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, name);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fremovexattr(int fd, const char *name)
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

	if (__strncmp(name, IAMROOT_XATTRS_PREFIX) != 0) {
		int ret;

		ret = _snprintf(xbuf, sizeof(xbuf), "%s%s",
				IAMROOT_XATTRS_PREFIX, name);
		if (ret == -1)
			return -1;

		name = xbuf;
	}

	__debug("%s(fd: %i <-> '%s', name: '%s' -> '%s', ...)\n", __func__,
		fd, buf, name, xbuf);

	return next_fremovexattr(fd, name);
}
#endif