/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __fpathperror2(int fd1, int fd2, const char *s)
{
	char buf1[PATH_MAX], buf2[PATH_MAX];
	const int save_errno = errno;
	ssize_t siz1, siz2;

	siz1 = fpath(fd1, buf1, sizeof(buf1));
	siz2 = fpath(fd2, buf2, sizeof(buf2));
	if (siz1 == -1 || siz2 == -1) {
#ifdef __FreeBSD__
		__notice("%s: %i, %i: %s: %i\n", __getrootdir(), fd1, fd2, s,
			 save_errno);
#else
		__notice("%s: %i, %i: %s: %s\n", __getrootdir(), fd1, fd2, s,
			 strerror(save_errno));
#endif
		errno = save_errno;
		return;
	}

	if (__ignored_errno(errno) || __ignored_function(s)) {
#ifdef __FreeBSD__
		__debug("%s: %i <-> %s, %i <-> %s: %s: %i\n", __getrootdir(),
			fd1, buf1, fd2, buf2, s, errno);
#else
		__debug("%s: %i <-> %s, %i <-> %s: %s: %m\n", __getrootdir(),
			fd1, buf1, fd2, buf2, s);
#endif
		return;
	}

#ifdef __FreeBSD__
	__note_or_fatal("%s: %i <-> %s, %i <-> %s: %s: %i\n", __getrootdir(),
			fd1, buf1, fd2, buf2, s, errno);
#else
	__note_or_fatal("%s: %i <-> %s, %i <-> %s: %s: %m\n", __getrootdir(),
			fd1, buf1, fd2, buf2, s);
#endif
}
