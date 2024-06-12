/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

static int (*sym)(int, int);

hidden int next_dup2(int oldfd, int newfd)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "dup2");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(oldfd, newfd);
}

int dup2(int oldfd, int newfd)
{
	int ret;

	ret = next_dup2(oldfd, newfd);

	if (ret >= 0)
		__setfd(newfd, __fpath(oldfd));

	__debug("%s(oldfd: %i <-> '%s', newfd: %i <-> '%s') -> %i\n",
		__func__, oldfd, __fpath(oldfd), newfd, __fpath2(newfd), ret);

	return ret;
}
