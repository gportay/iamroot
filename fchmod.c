/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
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
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, mode);
	if (ret == -1)
		__fperror(fd, __func__);

	return ret;
}

int fchmod(int fd, mode_t mode)
{
	__verbose("%s(fd: %i, mode: 0%03o)\n", __func__, fd, mode);
	__fwarn_if_insuffisant_user_mode(fd, mode);

	return next_fchmod(fd, mode);
}
