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
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, list, size);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

ssize_t listxattr(const char *path, char *list, size_t size)
{
	char xbuf[XATTR_NAME_MAX+1]; /* NULL-terminated */
	char buf[PATH_MAX];
	ssize_t xsize, siz;
	ssize_t i, ret;
	(void)size;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	xsize = next_listxattr(buf, xbuf, sizeof(xbuf)-1); /* NUL-terminated */
	if (xsize == -1)
		return -1;

	xbuf[xsize] = 0; /* ensure NULL-terminated */

	ret = 0;
	i = 0;
	do {
		size_t len, off = 0;

		len = __strlen(&xbuf[i]);
		if (!len)
			break;

		if (__strncmp(&xbuf[i], "user.iamroot.") == 0)
			off += sizeof("user.iamroot.")-1; /* NULL-terminated */

		if (list)
			strcpy(&list[ret], &xbuf[i+off]);

		i += len + 1; /* NULL-terminated */
		if (len != off)
			ret += len + 1 - off; /* NUL-terminated */
	} while (i < xsize);

	return ret;
}
#endif
