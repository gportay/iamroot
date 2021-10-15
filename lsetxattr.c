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

__attribute__((visibility("hidden")))
int next_lsetxattr(const char *path, const char *name, const void *value,
		  size_t size, int flags)
{
	int (*sym)(const char *, const char *, const void *, size_t, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "lsetxattr");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, name, value, size, flags);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
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

	__verbose("%s(path: '%s' -> '%s', name: '%s', ...)\n", __func__, path,
		  real_path, name);

	return next_lsetxattr(real_path, name, value, size, flags);
}
