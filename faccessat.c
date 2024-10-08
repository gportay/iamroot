/*
 * Copyright 2021-2024 Gaël PORTAY
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

hidden int next_faccessat(int dfd, const char *path, int mode, int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "faccessat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, mode, flags);
}

int faccessat(int dfd, const char *path, int mode, int flags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_faccessat(dfd, buf, mode, flags);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', mode: 0%03o, flags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, mode, flags, ret);

	return ret;
}
