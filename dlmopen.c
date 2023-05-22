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
static void *(*sym)(Lmid_t, const char *, int);

__attribute__((visibility("hidden")))
void *next_dlmopen(Lmid_t lmid, const char *path, int flags)

{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "dlmopen");

	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	return sym(lmid, path, flags);
}

void *dlmopen(Lmid_t lmid, const char *path, int flags)
{
	char buf[PATH_MAX];
	void *ret = NULL;
	ssize_t siz;

	if (!path || !__inchroot())
		return next_dlmopen(lmid, path, flags);

	if (!strchr(path, '/')) {
		siz = __path_access(path, F_OK, __library_path(), buf,
				    sizeof(buf));
		if (siz != -1)
			goto next;
	}

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

next:
	ret = next_dlmopen(lmid, buf, flags);

exit:
	__debug("%s(..., path: '%s' -> '%s', flags: 0x%x) -> %p\n", __func__,
		path, buf, flags, ret);

	return ret;
}
#endif
