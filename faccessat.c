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
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, mode, atflags);
}

int faccessat(int dfd, const char *path, int mode, int atflags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

	ret = next_faccessat(dfd, buf, mode, atflags);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', mode: 0%03o, atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, mode, atflags, ret);

	return ret;
}
