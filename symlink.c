/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
int next_symlink(const char *string, const char *path)
{
	int (*sym)(const char *, const char *);

	sym = dlsym(RTLD_NEXT, "symlink");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(string, path);
}

int symlink(const char *string, const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(string: '%s': path: '%s' -> '%s')\n", __func__, string,
		  path, real_path);

	return next_symlink(string, real_path);
}
