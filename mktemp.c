/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "iamroot.h"

static char *(*sym)(char *);

hidden char *next_mktemp(char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "mktemp");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(path);
}

char *mktemp(char *path)
{
	char buf[PATH_MAX];
	char *ret = NULL;
	ssize_t siz;
	size_t len;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_mktemp(buf);
	if (!*ret)
		goto exit;

	len = __strlen(__basename(path));
	strncpy(&path[__strlen(path)-len], &buf[__strlen(buf)-len], len);
	ret = path;

exit:
	__debug("%s(path: '%s' -> '%s') -> '%s'\n", __func__, path, buf, ret);

	return ret;
}
