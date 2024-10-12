/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include "iamroot.h"

/*
 * Stolen from musl (src/stat/futimesat.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/syscall.h>

int __futimesat(int dirfd, const char *pathname, const struct timeval times[2])
{
	struct timespec ts[2];
	if (times) {
		int i;
		for (i=0; i<2; i++) {
			if (times[i].tv_usec >= 1000000)
				return __syscall_ret(-EINVAL);
			ts[i].tv_sec = times[i].tv_sec;
			ts[i].tv_nsec = times[i].tv_usec * 1000;
		}
	}
	return utimensat(dirfd, pathname, times ? ts : 0, 0);
}

int futimesat(int dfd, const char *path, const struct timeval times[2])
{
	__debug("%s(dfd: %i <-> '%s', path: '%s', ...)\n", __func__, dfd,
		__fpath(dfd), path);

	/* Forward to another function */
	return __futimesat(dfd, path, times);
}

#ifdef __GLIBC__
#if __TIMESIZE == 32
#ifdef _LARGEFILE64_SOURCE
int __futimesat64 (int __fd, const char *__file,
		   const struct timeval __tvp[2]) __THROW;
weak_alias(futimesat, __futimesat64);
#endif
#endif
#endif
