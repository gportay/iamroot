/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/stat.h>
#include <fcntl.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next_creat64(const char *path, mode_t mode)
{
	int (*sym)(const char *, mode_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "creat64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, mode);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int creat64(const char *path, mode_t mode)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s', mode: %o)\n", __func__, path,
		  real_path, mode);

	return next_creat64(real_path, mode);
}
#endif
