/*
 * Copyright 2021-2023 Gaël PORTAY
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
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(path, pfx);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

char *tempnam(const char *path, const char *pfx)
{
	char buf[PATH_MAX];
	ssize_t siz;
	char *ret;

	if (!path)
		path = P_tmpdir;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, NULL);

	ret = next_tempnam(buf, pfx);
	if (!ret)
		goto exit;

	ret = __striprootdir(ret);

exit:
	__debug("%s(path: '%s' -> '%s', pfx: '%s') -> '%s'\n", __func__, path,
		buf, pfx, ret);

	return ret;
}
