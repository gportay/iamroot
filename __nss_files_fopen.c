/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
FILE *next___nss_files_fopen(const char * path)
{
	FILE *(*sym)(const char *);
	FILE *ret;

	sym = dlsym(RTLD_NEXT, "__nss_files_fopen");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

FILE *__nss_files_fopen(const char * path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next___nss_files_fopen(real_path);
}
#endif
