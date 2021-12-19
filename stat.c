/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

#ifndef __GLIBC_PREREQ
#define __GLIBC_PREREQ(maj,min) 0
#endif

extern int rootstat(const char *, struct stat *);

__attribute__((visibility("hidden")))
int next_stat(const char *path, struct stat *statbuf)
{
#if defined __GLIBC__ && !__GLIBC_PREREQ(2,33)
	int next___xstat(int, const char *, struct stat *);
	return next___xstat(_STAT_VER, path, statbuf);
#else
	int (*sym)(const char *, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "stat");
	if (!sym) {
		int next___xstat(int, const char *, struct stat *);
		__dl_perror(__func__);
		return next___xstat(0, path, statbuf);
	}

	ret = sym(path, statbuf);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
#endif
}

int stat(const char *path, struct stat *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return rootstat(real_path, statbuf);
}
