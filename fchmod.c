/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/stat.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fchmod(int fd, mode_t mode)
{
	int (*sym)(int, mode_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchmod");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, mode);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fchmod(int fd, mode_t mode)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, "__procfdreadlink");
		return -1;
	}
	buf[siz] = 0;

	__debug("%s(fd: %i <-> '%s', mode: 0%03o)\n", __func__, fd, buf, mode);
	__fwarn_if_insuffisant_user_mode(fd, mode);

	ret = next_fchmod(fd, mode);
	__ignore_error_and_warn(ret, AT_FDCWD, buf, 0);

	return ret;
}
