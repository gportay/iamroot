/*
 * Copyright 2021-2023 Gaël PORTAY
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
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(lmid, path, flags);
	if (!ret)
		__pathdlperror(path, __func__);

	return ret;
}

void *dlmopen(Lmid_t lmid, const char *path, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	if (!path || !__inchroot())
		return next_dlmopen(lmid, path, flags);

	if (!strchr(path, '/')) {
		siz = __path_access(path, F_OK, getenv("IAMROOT_LIBRARY_PATH"),
				    buf, sizeof(buf));
		if (siz != -1)
			goto next;
	}

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return NULL;
	}

next:
	__debug("%s(..., path: '%s' -> '%s', flags: 0x%x)\n", __func__, path,
		buf, flags);

	return next_dlmopen(lmid, buf, flags);
}
#endif
