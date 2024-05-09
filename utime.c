/*
 * Copyright 2021-2022,2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include <utime.h>

#include "iamroot.h"

/*
 * Stolen from musl (src/time/utime.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <utime.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

int utime(const char *path, const struct utimbuf *times)
{
	return utimensat(AT_FDCWD, path, times ? ((struct timespec [2]){
		{ .tv_sec = times->actime }, { .tv_sec = times->modtime }})
		: 0, 0);
}

#ifdef __GLIBC__
#if __TIMESIZE == 32
#ifdef _LARGEFILE64_SOURCE
int __utime64 (const char *__file,
	       const struct utimbuf *__file_times)
     __THROW __nonnull ((1));
weak_alias(utime, __utime64);
#endif
#endif
#endif
