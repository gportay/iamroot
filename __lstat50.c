/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __NetBSD__
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

int __lstat50(const char *path, struct stat *statbuf)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	/* Forward to another function */
	return fstatat(AT_FDCWD, path, statbuf, AT_SYMLINK_NOFOLLOW);
}
#endif
