/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

#ifdef _LARGEFILE64_SOURCE
extern int __fxstatat64(int, int, const char *, struct stat64 *, int);

int __lxstat64(int ver, const char *path, struct stat64 *statbuf)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	/* Forward to another function */
	return __fxstatat64(ver, AT_FDCWD, path, statbuf, AT_SYMLINK_NOFOLLOW);
}
#endif
#endif
