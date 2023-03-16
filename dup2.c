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
int next_dup2(int oldfd, int newfd)
{
	int (*sym)(int, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "dup2");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(oldfd, newfd);
	if (ret == -1)
		__fpathperror(oldfd, __func__);

	return ret;
}

int dup2(int oldfd, int newfd)
{
	__debug("%s(oldfd: %i <-> '%s', newfd: %i <-> '%s')\n", __func__,
		oldfd, __fpath(oldfd), newfd, __fpath2(newfd));

	return next_dup2(oldfd, newfd);
}
