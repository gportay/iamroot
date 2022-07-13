/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

#ifdef _LARGEFILE64_SOURCE
extern int __fxstatat64(int, int, const char *, struct stat64 *, int);

int __fxstat64(int ver, int fd, struct stat64 *statbuf)
{
	__debug("%s(fd: %i, ...)\n", __func__, fd);

	return __fxstatat64(ver, fd, "", statbuf, AT_EMPTY_PATH);
}
#endif
#endif
