/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __NetBSD__
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

static FILE *(*sym)(const char *, const char *);

hidden FILE *next_fopen(const char *path, const char *mode)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fopen");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(path, mode);
}

FILE *fopen(const char *path, const char *mode)
{
	const int errno_save = errno;
	char buf[PATH_MAX];
	FILE *ret = NULL;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_fopen(buf, mode);
	if (ret != NULL && __setfd(fileno(ret), buf))
		errno = errno_save;

exit:
	__debug("%s(path: '%s' -> '%s', mode: '%s') -> %p\n", __func__, path,
		buf, mode, ret);

	return ret;
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(fopen, fopen64);
#endif
#endif
