/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include <dlfcn.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void *next_dlopen(const char *path, int flags)

{
	void *(*sym)(const char *, int);
	void *ret;

	sym = dlsym(RTLD_NEXT, "dlopen");
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(path, flags);
	if (!ret)
		__pathdlperror(path, __func__);

	return ret;
}

void *dlopen(const char *path, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	void *ret;

	if (!path || !__inchroot())
		return next_dlopen(path, flags);

	if (!strchr(path, '/')) {
		siz = __path_access(path, F_OK, __library_path(), buf,
				    sizeof(buf));
		if (siz != -1)
			goto next;
	}

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, NULL);

next:
	ret = next_dlopen(buf, flags);

	__debug("%s(path: '%s' -> '%s', flags: 0x%x) -> %p\n", __func__, path,
		buf, flags, ret);

	return ret;
}
