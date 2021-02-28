/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__

__attribute__((visibility("hidden")))
int next___open_2(const char *path, int flags)
{
	int (*sym)(const char *, int);

	sym = dlsym(RTLD_NEXT, "__open_2");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(path, flags);
}

int __open_2(const char *path, int flags)
{
	const char *real_path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next___open_2(real_path, flags);
}
#endif
