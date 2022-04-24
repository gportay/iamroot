/*
 * Copyright 2021-2022 Gaël PORTAY
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

__attribute__((visibility("hidden")))
char *next_mktemp(char *path)
{
	char *(*sym)(char *);
	char *ret;

	sym = dlsym(RTLD_NEXT, "mktemp");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

char *mktemp(char *path)
{
	char buf[PATH_MAX];
	size_t len;
	char *ret;

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0) == -1) {
		__pathperror(path, __func__);
		return NULL;
	}

	ret = next_mktemp(buf);
	if (!*ret)
		goto exit;

	len = __strlen(path);
	memcpy(path, buf+__strlen(buf)-len, len);

exit:
	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return path;
}
