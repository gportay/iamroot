/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/xattr.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
int next_lremovexattr(const char *path, const char *name)
{
	int (*sym)(const char *, const char *);

	sym = dlsym(RTLD_NEXT, "lremovexattr");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(path, name);
}

int lremovexattr(const char *path, const char *name)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return next_lremovexattr(real_path, name);
}
