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
		int next___fxstat64(int, int, struct stat64 *);
		__dl_perror(__func__);
		return next___fxstat64(0, fd, statbuf);
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
#endif
}

int fstat64(int fd, struct stat64 *statbuf)
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

	__debug("%s(fd: %i <-> '%s', ...)\n", __func__, fd, real_path);

	return rootfstat64(fd, statbuf);
}
#endif
