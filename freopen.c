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
FILE *next_freopen(const char *path, const char *mode, FILE *stream)
{
	FILE *(*sym)(const char *, const char *, FILE *);
	FILE *ret;

	sym = dlsym(RTLD_NEXT, "freopen");
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(path, mode, stream);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

FILE *freopen(const char *path, const char *mode, FILE *stream)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, NULL);

	__debug("%s(path: '%s' -> '%s', mode: '%s', ...)\n", __func__, path,
		buf, mode);

	return next_freopen(buf, mode, stream);
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(freopen, freopen64);
#endif
