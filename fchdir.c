/*
 * Copyright 2021 Gaël PORTAY
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
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fchdir(int fd)
{
	char buf[PATH_MAX];
	char *real_path;
	ssize_t siz;
	int ret;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, "__procfdreadlink");
		return -1;
	}
	buf[siz] = 0; /* ensure NULL terminated */
	real_path = buf;

	ret = next_fchdir(fd);
	if (ret) {
		__fpathperror(fd, __func__);
		return ret;
	}

	__debug("%s(fd: %i <-> '%s')\n", __func__, fd, real_path);

	return chrootdir(NULL);
}
