/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

static char *(*sym)(int);

__attribute__((visibility("hidden")))
char *next_ttyname(int fd)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "ttyname");

	if (!sym)
		return __set_errno(ENOSYS, NULL);

	return sym(fd);
}

char *ttyname(int fd)
{
	char *ret;

	ret = next_ttyname(fd);
	if (!ret)
		goto exit;

	ret = __striprootdir(ret);

exit:
	__debug("%s(fd: %i <-> '%s') -> '%s'\n", __func__, fd, __fpath(fd),
		ret);

	return ret;
}
