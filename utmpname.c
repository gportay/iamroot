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

#include <utmp.h>

#include "iamroot.h"

static int (*sym)(const char *);

__attribute__((visibility("hidden")))
int next_utmpname(const char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "utmpname");

	if (!sym)
		return __set_errno(ENOSYS, -1);

	return sym(path);
}

int utmpname(const char *path)
{
	int ret, atflags = 0;
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_utmpname(buf);

	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
#endif
