/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_tempnam(const char *path, const char *pfx)
{
	char *(*sym)(const char *, const char *);
	char *ret;

	sym = dlsym(RTLD_NEXT, "tempnam");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path, pfx);
	if (!ret)
		__perror(path, __func__);

	return ret;
}

char *tempnam(const char *path, const char *pfx)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_tempnam(real_path, pfx);
}
