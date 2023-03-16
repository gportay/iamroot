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
int next_dup3(int oldfd, int newfd, int oflags)
{
	int (*sym)(int, int, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "dup3");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(oldfd, newfd, oflags);
	if (ret == -1)
		__fpathperror(oldfd, __func__);

	return ret;
}

int dup3(int oldfd, int newfd, int oflags)
{
	int ret;

	__debug("%s(oldfd: %i <-> '%s', newfd: %i <-> '%s', oflags: 0%o)\n",
		__func__, oldfd, __fpath(oldfd), newfd, __fpath2(newfd),
		oflags);

	ret = next_dup3(oldfd, newfd, oflags);

	if (ret >= 0)
		__notice("%s: %i -> '%s' -> %i -> '%s'\n", __func__, oldfd,
			 __fpath(oldfd), newfd, __fpath2(newfd));

	return ret;
}
