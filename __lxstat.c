/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

extern int __fxstatat(int, int, const char *, struct stat *, int);

int __lxstat(int ver, const char *path, struct stat *statbuf)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	/* Forward to another function */
	return __fxstatat(ver, AT_FDCWD, path, statbuf, AT_SYMLINK_NOFOLLOW);
}
