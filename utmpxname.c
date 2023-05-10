/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <utmpx.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_utmpxname(const char *path)
{
	int (*sym)(const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "utmpxname");
	if (!sym)
		return __set_errno(ENOSYS, -1);

	ret = sym(path);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int utmpxname(const char *path)
{
	char buf[PATH_MAX];
	int atflags = 0;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_utmpxname(buf);
}
#endif
