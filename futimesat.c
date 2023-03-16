/*
 * Copyright 2021-2023 Gaël PORTAY
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
	__debug("%s(dfd: %i, path: '%s', ...)\n", __func__, dfd, path);

	return __futimesat(dfd, path, times);
}
