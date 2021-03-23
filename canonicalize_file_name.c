/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
char *next_canonicalize_file_name(const char *path)
{
	char *(*sym)(const char *);

	sym = dlsym(RTLD_NEXT, "canonicalize_file_name");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(path);
}

char *canonicalize_file_name(const char *path)
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

	return next_canonicalize_file_name(real_path);
}
