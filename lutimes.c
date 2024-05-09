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
 * Stolen from musl (src/legacy/lutimes.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

int lutimes(const char *filename, const struct timeval tv[2])
{
	struct timespec times[2];
	if (tv) {
		times[0].tv_sec  = tv[0].tv_sec;
		times[0].tv_nsec = tv[0].tv_usec * 1000;
		times[1].tv_sec  = tv[1].tv_sec;
		times[1].tv_nsec = tv[1].tv_usec * 1000;
	}
	return utimensat(AT_FDCWD, filename, tv ? times : 0, AT_SYMLINK_NOFOLLOW);
}

#ifdef __GLIBC__
#if __TIMESIZE == 32
#ifdef _LARGEFILE64_SOURCE
int __lutimes64 (const char *__file, const struct timeval __tvp[2])
     __THROW __nonnull ((1));
weak_alias(lutimes, __lutimes64);
#endif
#endif
#endif
