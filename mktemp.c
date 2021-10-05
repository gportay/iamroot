/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_mktemp(char *path)
{
	char *(*sym)(char *);
	char *ret;

	sym = dlsym(RTLD_NEXT, "mktemp");
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

char *mktemp(char *path)
{
	char buf[PATH_MAX];
	char *real_path;
	size_t len;
	char *ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	ret = next_mktemp(real_path);
	if (!*ret)
		goto exit;

	len = strlen(path);
	memcpy(path, real_path+strlen(real_path)-len, len);

exit:
	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return path;
}
