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

extern int __rootfstat(int, struct stat *);

__attribute__((visibility("hidden")))
int next___fstat(int fd, struct stat *statbuf)
{
	int (*sym)(int, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__fstat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int __fstat(int fd, struct stat *statbuf)
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

	return __rootfstat(fd, statbuf);
}
