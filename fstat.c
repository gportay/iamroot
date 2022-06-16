/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/stat.h>

#include "iamroot.h"

extern int rootfstatat(int, const char *, struct stat *, int);

int fstat(int fd, struct stat *statbuf)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, "__procfdreadlink");
		return -1;
	}
	buf[siz] = 0;

	__debug("%s(fd: %i <-> %s, ...)\n", __func__, fd, buf);

	return rootfstatat(fd, "", statbuf, AT_EMPTY_PATH);
}

#ifdef __GLIBC__
weak_alias(fstat, __fstat);
#endif
