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

__attribute__((visibility("hidden")))
int next_close(int fd)
{
	int (*sym)(int);
	int ret;

	sym = dlsym(RTLD_NEXT, "close");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(fd);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int close(int fd)
{
	int ret;

	__debug("%s(fd: %i <-> '%s')\n", __func__, fd, __fpath(fd));

	ret = next_close(fd);

	if (ret != -1)
		__setfd(fd, NULL);

	return ret;
}
