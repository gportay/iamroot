/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_mkostemps(char *path, int suffixlen, int flags)
{
	int (*sym)(char *, int, int);

	sym = dlsym(RTLD_NEXT, "mkostemps");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, suffixlen, flags);
}

int mkostemps(char *path, int suffixlen, int flags)
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

	ret = next_mkostemps(real_path, suffixlen, flags);
	if (ret == -1)
		goto exit;

	len = strlen(path);
	memcpy(path, real_path+strlen(real_path)-len, len);

exit:
	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return ret;
}
