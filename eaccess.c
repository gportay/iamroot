/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_eaccess(const char *path, int mode)
{
	int (*sym)(const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "eaccess");
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

int eaccess(const char *path, int mode)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s', mode: 0%03o)\n", __func__, path,
		  real_path, mode);

	return next_eaccess(real_path, mode);
}
