/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
FILE *next_fopen64(const char *path, const char *mode)
{
	FILE *(*sym)(const char *, const char *);
	FILE *ret;

	sym = dlsym(RTLD_NEXT, "fopen64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path, mode);
	if (!ret)
		__perror(path, __func__);

	return ret;
}

FILE *fopen64(const char *path, const char *mode)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_fopen64(real_path, mode);
}
#endif
