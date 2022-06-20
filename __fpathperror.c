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
		__notice("%s: %i: %s: %s\n", getrootdir(), fd, s,
			 strerror(err));
		err = errno;
		return;
	}
	buf[siz] = 0; /* ensure NULL terminated */

	if ((errno != EPERM) && (errno != EACCES) && (errno != ENOSYS)) {
		__info("%s: %i <-> %s: %s: %m\n", getrootdir(), fd, buf, s);
		return;
	}

	__note_or_fatal("%s: %i <-> %s: %s: %m\n", getrootdir(), fd, buf, s);
}
