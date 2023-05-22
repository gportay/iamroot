/*
 * Copyright 2022-2023 Gaël PORTAY
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

static int (*sym)(int, const char *);

__attribute__((visibility("hidden")))
int next_fremovexattr(int fd, const char *name)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fremovexattr");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(fd, name);
}

int fremovexattr(int fd, const char *name)
{
	char xbuf[XATTR_NAME_MAX+1]; /* NULL-terminated */
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return __fpath_perror(fd, -1);

	if (!__strneq(name, IAMROOT_XATTRS_PREFIX)) {
		int ret;

		ret = _snprintf(xbuf, sizeof(xbuf), "%s%s",
				IAMROOT_XATTRS_PREFIX, name);
		if (ret == -1)
			return -1;

		name = xbuf;
	}

	ret = next_fremovexattr(fd, name);

	__debug("%s(fd: %i <-> '%s', name: '%s' -> '%s', ...) -> %i\n", __func__,
		fd, buf, name, xbuf, ret);

	return ret;
}
#endif
