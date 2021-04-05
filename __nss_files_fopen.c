/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__

__attribute__((visibility("hidden")))
FILE *next___nss_files_fopen(const char * path)
{
	FILE *(*sym)(const char *);

	sym = dlsym(RTLD_NEXT, "__nss_files_fopen");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(path);
}

FILE *__nss_files_fopen(const char * path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next___nss_files_fopen(real_path);
}
#endif
