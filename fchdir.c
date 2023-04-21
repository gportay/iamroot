/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fchdir(int fd)
{
	int (*sym)(int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchdir");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(fd);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fchdir(int fd)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return __fpath_perror(fd, -1);

	ret = next_fchdir(fd);
	if (ret == -1)
		return __fpath_perror(fd, -1);

	__debug("%s(fd: %i <-> '%s')\n", __func__, fd, buf);

	return __chrootdir(NULL);
}
