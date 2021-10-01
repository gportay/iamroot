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

#ifdef __GLIBC__

__attribute__((visibility("hidden")))
int next_mkostemps64(char *path, int suffixlen, int flags)
{
	int (*sym)(char *, int, int);

	sym = dlsym(RTLD_NEXT, "mkostemps64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(path, suffixlen, flags);
}

int mkostemps64(char *path, int suffixlen, int flags)
{
	char buf[PATH_MAX];
	char *real_path;
	size_t len;
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = next_mkostemps64(real_path, suffixlen, flags);
	if (ret == -1)
		goto exit;

	len = strlen(path);
	memcpy(path, real_path+strlen(real_path)-len, len);

exit:
	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return ret;
}
#endif
