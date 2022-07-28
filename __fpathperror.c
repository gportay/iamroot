/*
 * Copyright 2021-2022 Gaël PORTAY
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
	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
#ifdef __FreeBSD__
		__notice("%s: %i: %s: %i\n", getrootdir(), fd, s, errno);
#else
		__notice("%s: %i: %s: %s\n", getrootdir(), fd, s,
			 strerror(err));
#endif
		errno = err;
		return;
	}
	buf[siz] = 0; /* ensure NULL-terminated */

	if (__ignored_errno(errno)) {
#ifdef __FreeBSD__
		__info("%s: %i <-> %s: %s: %i\n", getrootdir(), fd, buf, s,
		       errno);
#else
		__info("%s: %i <-> %s: %s: %m\n", getrootdir(), fd, buf, s);
#endif
		return;
	}

#ifdef __FreeBSD__
	__note_or_fatal("%s: %i <-> %s: %s: %i\n", getrootdir(), fd, buf, s,
			errno);
#else
	__note_or_fatal("%s: %i <-> %s: %s: %m\n", getrootdir(), fd, buf, s);
#endif
}
