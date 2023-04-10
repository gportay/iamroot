/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __fpathperror(int fd, const char *s)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int err;

	err = errno;
	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1) {
#ifdef __FreeBSD__
		__notice("%s: %i: %s: %i\n", __getrootdir(), fd, s, errno);
#else
		__notice("%s: %i: %s: %s\n", __getrootdir(), fd, s,
			 strerror(err));
#endif
		errno = err;
		return;
	}

	if (__ignored_errno(errno)) {
#ifdef __FreeBSD__
		__info("%s: %i <-> %s: %s: %i\n", __getrootdir(), fd, buf, s,
		       errno);
#else
		__info("%s: %i <-> %s: %s: %m\n", __getrootdir(), fd, buf, s);
#endif
		return;
	}

#ifdef __FreeBSD__
	__note_or_fatal("%s: %i <-> %s: %s: %i\n", __getrootdir(), fd, buf, s,
			errno);
#else
	__note_or_fatal("%s: %i <-> %s: %s: %m\n", __getrootdir(), fd, buf, s);
#endif
}
