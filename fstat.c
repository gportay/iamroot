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

#ifndef __GLIBC_PREREQ
#define __GLIBC_PREREQ(maj,min) 0
#endif

extern int rootfstat(int, struct stat *);

__attribute__((visibility("hidden")))
int next_fstat(int fd, struct stat *statbuf)
{
#if defined __GLIBC__ && !__GLIBC_PREREQ(2,33)
	int next___fxstat(int, int, struct stat *);
	return next___fxstat(_STAT_VER, fd, statbuf);
#else
	int (*sym)(int, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstat");
	if (!sym) {
		int next___fxstat(int, int, struct stat *);
		__dl_perror(__func__);
		return next___fxstat(0, fd, statbuf);
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
#endif
}

int fstat(int fd, struct stat *statbuf)
{
	__debug("%s(fd: %i, ...)\n", __func__, fd);

	return rootfstat(fd, statbuf);
}
