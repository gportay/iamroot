/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/statvfs.h>

extern char *path_resolution(const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_statvfs(const char *path, struct statvfs *statvfsbuf)
{
	int (*sym)(const char *, struct statvfs *);

	sym = dlsym(RTLD_NEXT, "statvfs");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, statvfsbuf);
}

int statvfs(const char *path, struct statvfs *statvfsbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return next_statvfs(real_path, statvfsbuf);
}
