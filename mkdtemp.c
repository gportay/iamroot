/*
 * Copyright 2021-2023 Gaël PORTAY
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

__attribute__((visibility("hidden")))
char *next_mkdtemp(char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "mkdtemp");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(path);
}

char *mkdtemp(char *path)
{
	char buf[PATH_MAX];
	size_t len;
	char *ret;

	__strncpy(buf, path);
	ret = next_mkdtemp(buf);
	if (!*ret)
		goto exit;

	len = __strlen(path);
	memcpy(path, buf+__strlen(buf)-len, len);

exit:
	__debug("%s(path: '%s') -> '%s'\n", __func__, path, ret);

	return path;
}
