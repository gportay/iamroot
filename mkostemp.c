/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_mkostemp(char *path, int flags)
{
	int (*sym)(char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "mkostemp");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, flags);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int mkostemp(char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;
	size_t len;
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__perror(path, "path_resolution");
		return -1;
	}

	ret = next_mkostemp(real_path, flags);
	if (ret == -1)
		goto exit;

	len = strlen(path);
	memcpy(path, real_path+strlen(real_path)-len, len);

exit:
	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return ret;
}
