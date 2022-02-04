/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
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
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path, mode);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

FILE *fopen64(const char *path, const char *mode)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s', mode: '%s')\n", __func__, path,
		real_path, mode);

	return next_fopen64(real_path, mode);
}
#endif
