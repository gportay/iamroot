/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/stat.h>

#include "iamroot.h"

#ifdef __GLIBC__
extern int rootfstat64(int, struct stat64 *);

__attribute__((visibility("hidden")))
int next_fstat64(int fd, struct stat64 *statbuf)
{
	int (*sym)(int, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstat64");
	if (!sym) {
		int next___fxstat64(int, int, struct stat64 *);
#if defined(__arm__)
		return next___fxstat64(3, fd, statbuf);
#else
		return next___fxstat64(0, fd, statbuf);
#endif
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

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

	return rootfstat64(fd, statbuf);
}
#endif
