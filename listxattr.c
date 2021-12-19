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
ssize_t next_listxattr(const char *path, char *list, size_t size)
{
	ssize_t (*sym)(const char *, char *, size_t);
	ssize_t ret;

	sym = dlsym(RTLD_NEXT, "listxattr");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, list, size);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

ssize_t listxattr(const char *path, char *list, size_t size)
{
	char xbuf[XATTR_LIST_MAX + 1];
	char buf[PATH_MAX];
	char *real_path;
	ssize_t xsize;
	ssize_t i, ret;

	(void)size;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__perror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	xsize = next_listxattr(real_path, xbuf, sizeof(xbuf)-1);
	if (xsize == -1)
		return -1;

	xbuf[xsize] = 0; /* ensure NULL terminated */

	ret = 0;
	i = 0;
	do {
		size_t len, off = 0;

		len = strlen(&xbuf[i]);
		if (!len)
			break;

		if (__strncmp(&xbuf[i], "user.iamroot.") == 0)
			off += sizeof("user.iamroot.") - 1;

		if (list)
			strcpy(&list[ret], &xbuf[i+off]);

		i += len + 1;
		ret += len + 1 - off;
	} while (i < xsize);

	return ret;
}
