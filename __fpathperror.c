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
	const int save_errno = errno;
	char buf[PATH_MAX];
	ssize_t siz;
	(void)s;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1) {
#if defined __FreeBSD__ || defined __OpenBSD__
		__notice("%i: %s: %i\n", fd, s, save_errno);
#else
		__notice("%i: %s: %s\n", fd, s, strerror(save_errno));
#endif
		errno = save_errno;
		return;
	}

	if (__ignored_errno(errno)) {
#if defined __FreeBSD__ || defined __OpenBSD__
		__debug("%i <-> %s: %s: %i\n", fd, buf, s, errno);
#else
		__debug("%i <-> %s: %s: %m\n", fd, buf, s);
#endif
		return;
	}

#if defined __FreeBSD__ || defined __OpenBSD__
	__note_or_fatal("%i <-> %s: %s: %i\n", fd, buf, s, errno);
#else
	__note_or_fatal("%i <-> %s: %s: %m\n", fd, buf, s);
#endif
}
