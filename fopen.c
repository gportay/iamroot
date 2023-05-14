/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
FILE *next_fopen(const char *path, const char *mode)
{
	FILE *(*sym)(const char *, const char *);
	FILE *ret;

	sym = dlsym(RTLD_NEXT, "fopen");
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(path, mode);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

FILE *fopen(const char *path, const char *mode)
{
	char buf[PATH_MAX];
	ssize_t siz;
	FILE *ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, NULL);

	ret = next_fopen(buf, mode);

	if (ret != NULL)
		__setfd(fileno(ret), buf);

	__debug("%s(path: '%s' -> '%s', mode: '%s') -> %p\n", __func__, path,
		buf, mode, ret);

	return ret;
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(fopen, fopen64);
#endif
