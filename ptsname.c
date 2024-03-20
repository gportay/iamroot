/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "iamroot.h"

static char *(*sym)(int);

hidden
char *next_ptsname(int fd)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "ptsname");

	if (!sym)
		return __set_errno(ENOSYS, NULL);

	return sym(fd);
}

char *ptsname(int fd)
{
	char *ret;

	ret = next_ptsname(fd);
	if (!ret)
		goto exit;

	ret = __striprootdir(ret);

exit:
	__debug("%s(fd: %i <-> '%s') -> '%s'\n", __func__, fd, __fpath(fd),
		ret);

	return ret;
}
