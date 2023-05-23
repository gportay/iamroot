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

static FILE *(*sym)(const char *, const char *, FILE *);

__attribute__((visibility("hidden")))
FILE *next_freopen(const char *path, const char *mode, FILE *stream)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "freopen");

	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	return sym(path, mode, stream);
}

FILE *freopen(const char *path, const char *mode, FILE *stream)
{
	char buf[PATH_MAX];
	ssize_t siz;
	FILE *ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, NULL);

	ret = next_freopen(buf, mode, stream);

	if (ret != NULL)
		__setfd(fileno(ret), buf);

	__debug("%s(path: '%s' -> '%s', mode: '%s', ...) -> %p\n", __func__,
		path, buf, mode, ret);

	return ret;
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(freopen, freopen64);
#endif
