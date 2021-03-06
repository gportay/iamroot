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

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_mkstemps64(char *path, int suffixlen)
{
	int (*sym)(char *, int);

	sym = dlsym(RTLD_NEXT, "mkstemps64");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, suffixlen);
}

int mkstemps64(char *path, int suffixlen)
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

	ret = next_mkstemps64(real_path, suffixlen);
	if (ret == -1)
		goto exit;

	len = strlen(path);
	memcpy(path, real_path+strlen(real_path)-len, len);

exit:
	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return ret;
}
#endif
