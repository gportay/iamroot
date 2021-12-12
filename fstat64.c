/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

#ifdef __GLIBC__
int fstat64(int fd, struct stat64 *statbuf)
{
	__debug("%s(fd: %i, ...)\n", __func__, fd);

	return fstatat64(fd, "", statbuf, AT_EMPTY_PATH);
}

weak_alias(fstat64, __fstat64);
weak_alias(fstat64, __fstat_time64);
#endif
