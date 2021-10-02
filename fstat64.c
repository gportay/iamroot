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

#ifdef __GLIBC__
extern int rootfstat64(int, struct stat64 *);

__attribute__((visibility("hidden")))
int next_fstat64(int fd, struct stat64 *statbuf)
{
#if defined __GLIBC__ && !__GLIBC_PREREQ(2,33)
	int next___fxstat64(int, int, struct stat64 *);
	return next___fxstat64(_STAT_VER, fd, statbuf);
#else
	int (*sym)(int, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstat64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fperror(fd, __func__);

	return ret;
#endif
}

int fstat64(int fd, struct stat64 *statbuf)
{
	__verbose("%s(fd: %i, ...)\n", __func__, fd);

	return rootfstat64(fd, statbuf);
}
#endif
