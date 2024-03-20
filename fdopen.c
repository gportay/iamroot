/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <stdio.h>

#include "iamroot.h"

static FILE *(*sym)(int, const char *);

hidden FILE *next_fdopen(int fd, const char *mode)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fdopen");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(fd, mode);
}

FILE *fdopen(int fd, const char *mode)
{
	FILE *ret;

	ret = next_fdopen(fd, mode);

	__debug("%s(fd: %i <-> '%s', mode: '%s') -> %p\n", __func__, fd,
		__fpath(fd), mode, ret);

	return ret;
}
