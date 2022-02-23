/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

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
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(lmid, path, flags);
	if (!ret)
		__pathdlperror(path, __func__);

	return ret;
}

void *dlmopen(Lmid_t lmid, const char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	if (!path || !inchroot())
		return next_dlmopen(lmid, path, flags);

	if (!strchr(path, '/')) {
		real_path = path_access(path, F_OK,
					getenv("IAMROOT_LD_LIBRARY_PATH"), buf,
					sizeof(buf));
		if (real_path)
			goto next;
	}

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, __func__);
		return NULL;
	}

next:
	__debug("%s(..., path: '%s' -> '%s', flags: 0x%x)\n", __func__, path,
		real_path, flags);

	return next_dlmopen(lmid, real_path, flags);
}
#endif
