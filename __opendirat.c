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

#include "fpath_resolutionat.h"

#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
DIR *next___opendirat(int fd, const char *path)
{
	DIR *(*sym)(int, const char *);

	sym = dlsym(RTLD_NEXT, "__opendirat");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(fd, path);
}

DIR *__opendirat(int fd, const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return NULL;
	}

	__fprintf(stderr, "%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd,
			  path, real_path);

	return next___opendirat(fd, real_path);
}
#endif
