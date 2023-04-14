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

int fstat(int fd, struct stat *statbuf)
{
	__debug("%s(fd: %i <-> '%s', ...)\n", __func__, fd, __fpath(fd));

	return fstatat(fd, "", statbuf, AT_EMPTY_PATH);
}

#ifdef __GLIBC__
weak_alias(fstat, __fstat);
#endif
