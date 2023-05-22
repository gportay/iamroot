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

static int (*sym)(int, const char *, int, int);

__attribute__((visibility("hidden")))
int next_faccessat(int dfd, const char *path, int mode, int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "faccessat");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(dfd, path, mode, atflags);
}

int faccessat(int dfd, const char *path, int mode, int atflags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_faccessat(dfd, buf, mode, atflags);

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', mode: 0%03o, atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, mode, atflags, ret);

	return ret;
}
