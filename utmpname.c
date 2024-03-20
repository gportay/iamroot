/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined(__linux__) || defined(__NetBSD__)
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <utmp.h>

#include "iamroot.h"

static int (*sym)(const char *);

hidden int next_utmpname(const char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "utmpname");

	if (!sym)
		return __set_errno(ENOSYS, -1);

	return sym(path);
}

int utmpname(const char *path)
{
	int atflags = 0, ret = -1;
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

	ret = next_utmpname(buf);

exit:
	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
#endif
