/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <dlfcn.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
void *next_dlmopen(Lmid_t lmid, const char *path, int flags)

{
	void *(*sym)(Lmid_t, const char *, int);
	void *ret;

	sym = dlsym(RTLD_NEXT, "dlmopen");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(lmid, path, flags);
	if (!ret)
		__perror(path, __func__);

	return ret;
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

	__verbose_func("%s(..., path: '%s' -> '%s')\n", __func__, path,
		       real_path);

	return next_dlmopen(lmid, path, flags);
}

#endif
