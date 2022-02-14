/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <dirent.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
DIR *next_open64dir(const char *path)
{
	DIR *(*sym)(const char *);
	DIR *ret;

	sym = dlsym(RTLD_NEXT, "open64dir");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

DIR *open64dir(const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, __func__);
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_open64dir(real_path);
}
