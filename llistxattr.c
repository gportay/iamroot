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

extern char *path_resolution(const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
ssize_t next_llistxattr(const char *path, char *list, size_t size)
{
	ssize_t (*sym)(const char *, char *, size_t);

	sym = dlsym(RTLD_NEXT, "llistxattr");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, list, size);
}

ssize_t llistxattr(const char *path, char *list, size_t size)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return next_llistxattr(real_path, list, size);
}
