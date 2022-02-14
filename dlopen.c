/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
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
	char *real_path;

	if (!path) {
		real_path = NULL;
		goto next;
	}

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, __func__);
		return NULL;
	}

next:
	__debug("%s(path: '%s' -> '%s', flags: 0x%x)\n", __func__, path,
		real_path, flags);

	return next_dlopen(real_path, flags);
}
