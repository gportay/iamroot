/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/stat.h>

#include "iamroot.h"

extern int __rootfxstat(int, int, struct stat *);

__attribute__((visibility("hidden")))
int next___fxstat(int ver, int fd, struct stat *statbuf)
{
	int (*sym)(int, int, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__fxstat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(ver, fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int __fxstat(int ver, int fd, struct stat *statbuf)
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

	return __rootfxstat(ver, fd, statbuf);
}
