/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <dlfcn.h>

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
void *next_dlmopen(Lmid_t lmid, const char *path, int flags)

{
	void *(*sym)(Lmid_t, const char *, int);

	sym = dlsym(RTLD_NEXT, "dlmopen");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(lmid, path, flags);
}

void *dlmopen(Lmid_t lmid, const char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__fprintf(stderr, "%s(..., path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_dlmopen(lmid, path, flags);
}

#endif
