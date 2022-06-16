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

#ifdef __GLIBC__
extern int rootfstatat64(int, const char *, struct stat64 *, int);

int fstat64(int fd, struct stat64 *statbuf)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, "__procfdreadlink");
		return -1;
	}
	buf[siz] = 0;

	__debug("%s(fd: %i <-> '%s', ...)\n", __func__, fd, buf);

	return rootfstatat64(fd, "", statbuf, AT_EMPTY_PATH);
}

weak_alias(fstat64, __fstat64);
weak_alias(fstat64, __fstat_time64);
#endif
