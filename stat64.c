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
int stat64(const char *path, struct stat64 *statbuf)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	/* Forward to another function */
	return fstatat64(AT_FDCWD, path, statbuf, 0);
}

int __stat64 (const char *__restrict __file,
	      struct stat64 *__restrict __buf) __THROW __nonnull ((1, 2));
weak_alias(stat64, __stat64);
int __stat_time64 (const char *__restrict __file,
		   struct stat64 *__restrict __buf) __THROW __nonnull ((1, 2));
weak_alias(stat64, __stat_time64);
#endif
#endif
