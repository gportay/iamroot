/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __NetBSD__
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

static char *(*sym)(const char *, const char *);

hidden char *next_tempnam(const char *path, const char *pfx)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "tempnam");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(path, pfx);
}

char *tempnam(const char *path, const char *pfx)
{
	char buf[PATH_MAX];
	char *ret = NULL;
	ssize_t siz;

	if (!path)
		path = P_tmpdir;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_tempnam(buf, pfx);
	if (!ret)
		goto exit;

	ret = __striprootdir(ret);

exit:
	__debug("%s(path: '%s' -> '%s', pfx: '%s') -> '%s'\n", __func__, path,
		buf, pfx, ret);

	return ret;
}
#endif
