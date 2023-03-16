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
ssize_t next_flistxattr(int fd, char *list, size_t size)
{
	ssize_t (*sym)(int, char *, size_t);
	ssize_t ret;

	sym = dlsym(RTLD_NEXT, "flistxattr");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, list, size);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

ssize_t flistxattr(int fd, char *list, size_t size)
{
	char xbuf[XATTR_LIST_MAX+1]; /* NULL-terminated */
	char buf[PATH_MAX];
	ssize_t xsize, siz;
	ssize_t i, ret;
	(void)size;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, __func__);
		return -1;
	}

	__debug("%s(fd: %i <-> '%s', ...)\n", __func__, fd, buf);

	xsize = next_flistxattr(fd, xbuf, sizeof(xbuf)-1); /* NULL-terminated */
	if (xsize == -1)
		return -1;

	xbuf[xsize] = 0; /* ensure NULL-terminated */

	ret = 0;
	i = 0;
	do {
		size_t len, off = 0;

		len = strnlen(&xbuf[i], sizeof(xbuf)-i);
		if (!len)
			break;

		if (__strncmp(&xbuf[i], IAMROOT_XATTRS_PREFIX) == 0)
			off += sizeof(IAMROOT_XATTRS_PREFIX)-1; /* NULL-terminated */

		if (list)
			strcpy(&list[ret], &xbuf[i+off]);

		i += len + 1; /* NULL-terminated */
		if (len != off)
			ret += len + 1 - off; /* NULL-terminated */
	} while (i < xsize);

	return ret;
}
#endif
