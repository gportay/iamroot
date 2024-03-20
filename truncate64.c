/*
 * Copyright 2022-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>
#include <sys/types.h>

#include "iamroot.h"

#ifdef __GLIBC__
static int (*sym)(const char *, off_t);

hidden int next_truncate64(const char *path, off64_t length)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "truncate64");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, length);
}

int truncate64(const char *path, off64_t length)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_truncate64(buf, length);

exit:
	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}
#endif
