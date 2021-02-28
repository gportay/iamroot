/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);
extern int rootstat(const char *, struct stat *);

__attribute__((visibility("hidden")))
int next_stat(const char *path, struct stat *statbuf)
{
	int (*sym)(const char *, struct stat *);

	sym = dlsym(RTLD_NEXT, "stat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(path, statbuf);
}

int stat(const char *path, struct stat *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = rootstat(real_path, statbuf);

	__verbose("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return ret;
}
