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
int next_chdir(const char *path)
{
	int (*sym)(const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "chdir");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int chdir(const char *path)
{
	char buf[PATH_MAX];
	char *real_path;
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__perror(path, "path_resolution");
		return -1;
	}

	ret = next_chdir(real_path);
	if (ret) {
		__perror(real_path, "chdir");
		return ret;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return chrootdir(NULL);
}
