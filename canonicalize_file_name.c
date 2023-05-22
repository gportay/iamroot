/*
 * Copyright 2021-2023 Gaël PORTAY
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
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

char *canonicalize_file_name(const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;
	char *ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, NULL);

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	ret = next_canonicalize_file_name(buf);
	if (!ret)
		return NULL;

	ret = __striprootdir(ret);
	if (!ret) {
		__pathperror(buf, __func__);
		return NULL;
	}

	return ret;
}
