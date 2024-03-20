/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

static int (*sym)(int);

hidden int next_fchdir(int fd)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fchdir");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd);
}

int fchdir(int fd)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		goto exit;

	ret = next_fchdir(fd);
	if (ret == -1)
		goto exit;

	ret = __chrootdir(NULL);

exit:
	__debug("%s(fd: %i <-> '%s') -> %i\n", __func__, fd, buf, ret);

	return ret;
}
