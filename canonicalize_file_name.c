/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
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
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

char *canonicalize_file_name(const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_canonicalize_file_name(real_path);
}
