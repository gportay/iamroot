/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_canonicalize_file_name(const char *path)
{
	char *(*sym)(const char *);
	char *ret;

	sym = dlsym(RTLD_NEXT, "canonicalize_file_name");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path);
	if (!ret)
		__perror(path, __func__);

	return ret;
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

	__verbose_func("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_canonicalize_file_name(real_path);
}
