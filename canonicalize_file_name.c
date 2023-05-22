/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "iamroot.h"

static char *(*sym)(const char *);

__attribute__((visibility("hidden")))
char *next_canonicalize_file_name(const char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "canonicalize_file_name");

	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	return sym(path);
}

char *canonicalize_file_name(const char *path)
{
	char buf[PATH_MAX];
	char *ret = NULL;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_canonicalize_file_name(buf);
	if (!ret)
		goto exit;

	ret = __striprootdir(ret);

exit:
	__debug("%s(path: '%s' -> '%s') -> '%s'\n", __func__, path, buf, ret);

	return ret;
}
