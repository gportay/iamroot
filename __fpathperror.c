/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __fpathperror(int fd, const char *s)
{
	char buf[PATH_MAX];
	char *real_path;
	ssize_t siz;
	int err;

	if ((errno != EPERM) && (errno != EACCES))
		return;

	err = errno;
	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		__notice("%i: %s: %s\n", fd, s, strerror(err));
		return;
	}
	buf[siz] = 0; /* ensure NULL terminated */
	real_path = buf;

	__notice("%i <-> %s: %s: %m\n", fd, real_path, s);
	if (__getfatal())
		raise(SIGABRT);
}
