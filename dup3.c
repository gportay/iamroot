/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

static int (*sym)(int, int, int);

hidden int next_dup3(int oldfd, int newfd, int oflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "dup3");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(oldfd, newfd, oflags);
}

int dup3(int oldfd, int newfd, int oflags)
{
	const int errno_save = errno;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = fpath(oldfd, buf, sizeof(buf));
	if (siz == -1 && __strncpy(buf, "(null)"))
		errno = errno_save;

	ret = next_dup3(oldfd, newfd, oflags);
	if (ret >= 0 && __setfd(ret, __fpath(oldfd)))
		errno = errno_save;

	__debug("%s(oldfd: %i <-> '%s', newfd: %i <-> '%s', oflags: 0%o) -> %i\n",
		__func__, oldfd, buf, newfd, __fpath2(newfd), oflags, ret);

	return ret;
}
