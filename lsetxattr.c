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
int next_lsetxattr(const char *path, const char *name, const void *value,
		  size_t size, int flags)
{
	int (*sym)(const char *, const char *, const void *, size_t, int);

	sym = dlsym(RTLD_NEXT, "lsetxattr");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, name, value, size, flags);
}

int lsetxattr(const char *path, const char *name, const void *value,
	     size_t size, int flags)
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

	return next_lsetxattr(real_path, name, value, size, flags);
}
