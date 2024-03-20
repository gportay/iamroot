/*
 * Copyright 2021-2024 Gaël PORTAY
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

hidden
FILE *next_freopen(const char *path, const char *mode, FILE *stream)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "freopen");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(path, mode, stream);
}

FILE *freopen(const char *path, const char *mode, FILE *stream)
{
	char buf[PATH_MAX];
	FILE *ret = NULL;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_freopen(buf, mode, stream);

	if (ret != NULL)
		__setfd(fileno(ret), buf);

exit:
	__debug("%s(path: '%s' -> '%s', mode: '%s', ...) -> %p\n", __func__,
		path, buf, mode, ret);

	return ret;
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(freopen, freopen64);
#endif
