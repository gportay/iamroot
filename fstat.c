/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

int fstat(int fd, struct stat *statbuf)
{
	__debug("%s(fd: %i, ...)\n", __func__, fd);

	return fstatat(fd, "", statbuf, AT_EMPTY_PATH);
}

#ifdef __GLIBC__
weak_alias(fstat, __fstat);
#endif
