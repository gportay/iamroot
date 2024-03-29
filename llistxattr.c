/*
 * Copyright 2021-2024 Gaël PORTAY
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

static ssize_t (*sym)(const char *, char *, size_t);

hidden ssize_t next_llistxattr(const char *path, char *list, size_t size)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "llistxattr");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, list, size);
}

ssize_t llistxattr(const char *path, char *list, size_t size)
{
	ssize_t i, xsize, siz, ret = -1;
	char xbuf[XATTR_LIST_MAX+1]; /* NULL-terminated */
	char buf[PATH_MAX];
	(void)size;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	ret = next_llistxattr(buf, xbuf, sizeof(xbuf)-1); /* NULL-terminated */
	if (ret == -1)
		goto exit;

	xsize = ret;
	xbuf[xsize] = 0; /* ensure NULL-terminated */

	ret = 0;
	i = 0;
	do {
		size_t len, off = 0;

		len = strnlen(&xbuf[i], sizeof(xbuf)-i);
		if (!len)
			break;

		if (__strneq(&xbuf[i], IAMROOT_XATTRS_PREFIX))
			off += sizeof(IAMROOT_XATTRS_PREFIX)-1; /* NULL-terminated */

		if (list)
			strcpy(&list[ret], &xbuf[i+off]);

		i += len + 1; /* NULL-terminated */
		if (len != off)
			ret += len + 1 - off; /* NULL-terminated */
	} while (i < xsize);

exit:
	__debug("%s(path: '%s' -> '%s', ...) -> %zi\n", __func__, path, buf,
		ret);

	return ret;
}
#endif
