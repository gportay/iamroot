/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

static int (*sym)(const char *, int, const char *);

__attribute__((visibility("hidden")))
int next_symlinkat(const char *string, int dfd, const char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "symlinkat");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(string, dfd, path);
}

int symlinkat(const char *string, int dfd, const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_symlinkat(string, dfd, buf);

	__debug("%s(string: '%s', dfd: %i <-> '%s', path: '%s' -> '%s') -> %i\n",
		__func__, string, dfd, __fpath(dfd), path, buf, ret);

	return ret;
}
