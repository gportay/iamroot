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

__attribute__((visibility("hidden")))
void *next_dlopen(const char *path, int flags)

{
	void *(*sym)(const char *, int);
	void *ret;

	sym = dlsym(RTLD_NEXT, "dlopen");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path, flags);
	if (!ret)
		__pathdlperror(path, __func__);

	return ret;
}

void *dlopen(const char *path, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	if (!path || !inchroot())
		return next_dlopen(path, flags);

	if (!strchr(path, '/'))
		if (path_access(path, F_OK, getenv("IAMROOT_LIBRARY_PATH"),
				buf, sizeof(buf)) == -1)
			goto next;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return NULL;
	}

next:
	__debug("%s(path: '%s' -> '%s', flags: 0x%x)\n", __func__, path,
		buf, flags);

	return next_dlopen(buf, flags);
}
