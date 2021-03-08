/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifdef __GLIBC__

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <dlfcn.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...);

void *next_dlmopen(Lmid_t lmid, const char *path, int flags)

{
	void *(*sym)(Lmid_t, const char *, int);

	sym = dlsym(RTLD_NEXT, "dlmopen");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(lmid, path, flags);
}

void *dlmopen(Lmid_t lmid, const char *path, int flags)
{
	const char *real_path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__fprintf(stderr, "%s(..., path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_dlmopen(lmid, path, flags);
}

#endif
