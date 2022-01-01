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

extern int rootfstat(int, struct stat *);

__attribute__((visibility("hidden")))
int next_fstat(int fd, struct stat *statbuf)
{
	int (*sym)(int, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstat");
	if (!sym) {
		int next___fxstat(int, int, struct stat *);
#if defined(__arm__)
		return next___fxstat(3, fd, statbuf);
#else
		return next___fxstat(0, fd, statbuf);
#endif
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fstat(int fd, struct stat *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;
	ssize_t siz;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, "__procfdreadlink");
		return -1;
	}
	buf[siz] = 0;
	real_path = buf;

	__debug("%s(fd: %i <-> %s, ...)\n", __func__, fd, real_path);

	return rootfstat(fd, statbuf);
}
