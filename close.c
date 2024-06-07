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

static int (*sym)(int);

hidden int next_close(int fd)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "close");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd);
}

int close(int fd)
{
	const int errno_save = errno;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1 && __strncpy(buf, "(null)"))
		errno = errno_save;

	ret = next_close(fd);
	if (ret != -1 && __unsetfd(fd))
		errno = errno_save;

	__debug("%s(fd: %i <-> '%s') -> %i\n", __func__, fd, buf, ret);

	return ret;
}
