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

static int (*sym)(int, int, int);

hidden
int next_dup3(int oldfd, int newfd, int oflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "dup3");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(oldfd, newfd, oflags);
}

int dup3(int oldfd, int newfd, int oflags)
{
	int ret;

	ret = next_dup3(oldfd, newfd, oflags);

	if (ret >= 0)
		__setfd(newfd, __fpath(oldfd));

	__debug("%s(oldfd: %i <-> '%s', newfd: %i <-> '%s', oflags: 0%o) -> %i \n",
		__func__, oldfd, __fpath(oldfd), newfd, __fpath2(newfd),
		oflags, ret);

	return ret;
}
