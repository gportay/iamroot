/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

static int (*sym)(const char *, int);

__attribute__((visibility("hidden")))
int next_access(const char *path, int mode)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "access");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, mode);
}

int access(const char *path, int mode)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_access(buf, mode);

exit:
	__debug("%s(path: '%s' -> '%s', mode: 0%03o) -> %i\n", __func__, path,
		buf, mode, ret);

	return ret;
}
