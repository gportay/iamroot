/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

static int (*sym)(int);

__attribute__((visibility("hidden")))
int next_dup(int fd)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "dup");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(fd);
}

int dup(int fd)
{
	int ret;

	ret = next_dup(fd);

	if (ret >= 0)
		__setfd(ret, __fpath(fd));

	__debug("%s(fd: %i <-> '%s') -> %i\n", __func__, fd, __fpath(fd), ret);

	return ret;
}
