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
int stat64(const char *path, struct stat64 *statbuf)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	return fstatat64(AT_FDCWD, path, statbuf, 0);
}

weak_alias(stat64, __stat64);
weak_alias(stat64, __stat_time64);
#endif
