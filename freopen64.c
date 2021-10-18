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
FILE *next_freopen64(const char *path, const char *mode, FILE *stream)
{
	FILE *(*sym)(const char *, const char *, FILE *);
	FILE *ret;

	sym = dlsym(RTLD_NEXT, "freopen64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path, mode, stream);
	if (!ret)
		__perror(path, __func__);

	return ret;
}

FILE *freopen64(const char *path, const char *mode, FILE *stream)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_freopen64(real_path, mode, stream);
}
#endif
