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

#include <limits.h>
#include <stdlib.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern const char *getrootdir();

__attribute__((visibility("hidden")))
char *next_realpath(const char *path, char *resolved_path)
{
	char *(*sym)(const char *, char *);

	sym = dlsym(RTLD_NEXT, "realpath");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(path, resolved_path);
}

char *realpath(const char *path, char *resolved_path)
{
	const char *real_path;
	char buf[PATH_MAX];
	size_t len;
	char *ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	ret = next_realpath(real_path, resolved_path);
	if (!ret)
		return ret;

	len = strlen(getrootdir());
	if (len < 2)
		return ret;

	if (!resolved_path) {
		resolved_path = strdup(ret+len);
		free(ret);
		ret = resolved_path;
		return ret;
	}

	return memmove(resolved_path, resolved_path+len, strlen(ret)-len+1);
}
