/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
char *next_mkdtemp(char *path)
{
	char *(*sym)(char *);

	sym = dlsym(RTLD_NEXT, "mkdtemp");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(path);
}

char *mkdtemp(char *path)
{
	char buf[PATH_MAX];
	char *real_path;
	size_t len;
	char *ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	ret = next_mkdtemp(real_path);
	if (!*ret)
		goto exit;

	len = strlen(path);
	memcpy(path, real_path+strlen(real_path)-len, len);

exit:
	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return path;
}
