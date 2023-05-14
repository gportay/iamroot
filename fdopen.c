/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
FILE *next_fdopen(int fd, const char *mode)
{
	FILE *(*sym)(int, const char *);
	FILE *ret;

	sym = dlsym(RTLD_NEXT, "fdopen");
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(fd, mode);
	if (!ret)
		__fpathperror(fd, __func__);

	return ret;
}

FILE *fdopen(int fd, const char *mode)
{
	FILE *ret;

	ret = next_fdopen(fd, mode);

	__debug("%s(fd: %i <-> '%s', mode: '%s') -> %p\n", __func__, fd,
		__fpath(fd), mode, ret);

	return ret;
}
