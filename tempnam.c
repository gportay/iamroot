/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_tempnam(const char *path, const char *pfx)
{
	char *(*sym)(const char *, const char *);
	char *ret;

	sym = dlsym(RTLD_NEXT, "tempnam");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path, pfx);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

char *tempnam(const char *path, const char *pfx)
{
	char buf[PATH_MAX];

	if (!path)
		return next_tempnam(path, pfx);

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0) == -1) {
		__pathperror(path, __func__);
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s', pfx: '%s')\n", __func__, path, buf,
		pfx);

	return next_tempnam(buf, pfx);
}
