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

int __fxstat(int ver, int dfd, struct stat *statbuf)
{
	__debug("%s(dfd: %i <-> '%s', ...)\n", __func__, dfd, __fpath(dfd));

	return __fxstatat(ver, dfd, "", statbuf, AT_EMPTY_PATH);
}
