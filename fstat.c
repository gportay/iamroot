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

extern int rootfstat(int, struct stat *);

__attribute__((visibility("hidden")))
int next_fstat(int fd, struct stat *statbuf)
{
	int (*sym)(int, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstat");
	if (!sym) {
		int next___fxstat(int, int, struct stat *);
		__dlperror(__func__);
		return next___fxstat(0, fd, statbuf);
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fstat(int fd, struct stat *statbuf)
{
	__debug("%s(fd: %i, ...)\n", __func__, fd);

	return rootfstat(fd, statbuf);
}
