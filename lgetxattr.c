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
ssize_t next_lgetxattr(const char *path, const char *name, void *value,
		      size_t size)
{
	ssize_t (*sym)(const char *, const char *, void *, size_t);
	ssize_t ret;

	sym = dlsym(RTLD_NEXT, "lgetxattr");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, name, value, size);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

ssize_t lgetxattr(const char *path, const char *name, void *value, size_t size)
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

	return next_lgetxattr(real_path, name, value, size);
}
