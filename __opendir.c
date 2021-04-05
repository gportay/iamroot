/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <dirent.h>

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
DIR *next___opendir(const char *path)
{
	DIR *(*sym)(const char *);

	sym = dlsym(RTLD_NEXT, "__opendir");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(path);
}

DIR *__opendir(const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next___opendir(real_path);
}
#endif
