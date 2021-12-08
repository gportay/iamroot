/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>

#include "iamroot.h"

void __fperror(int fd, const char *s)
{
	char buf[PATH_MAX];
	char *real_path;
	ssize_t siz;

	if ((errno != EPERM) && (errno != EACCES))
		return;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		perror("__procfdreadlink");
		return;
	}
	buf[siz] = 0;
	real_path = buf;

	__notice("%i -> %s: %s: %m\n", fd, real_path, s);
	if (__fatal())
		raise(SIGABRT);
}
