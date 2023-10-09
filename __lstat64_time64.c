/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

#ifdef __GLIBC__
#ifdef _LARGEFILE64_SOURCE
#if __TIMESIZE == 32
extern int __fstatat64_time64(int, const char *, struct stat64 *, int);

int __lstat64_time64(const char *path, struct stat64 *statbuf)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	/* Forward to another function */
	return __fstatat64_time64(AT_FDCWD, path, statbuf,
				  AT_SYMLINK_NOFOLLOW);
}
#endif
#endif
#endif
#endif
