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
int fstat64(int fd, struct stat64 *statbuf)
{
	__debug("%s(fd: %i <-> '%s', ...)\n", __func__, fd, __fpath(fd));

	return fstatat64(fd, "", statbuf, AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW);
}

weak_alias(fstat64, __fstat64);
weak_alias(fstat64, __fstat_time64);
#endif
#endif
