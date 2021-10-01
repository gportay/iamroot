/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fchdir(int fd)
{
	int (*sym)(int);

	sym = dlsym(RTLD_NEXT, "fchdir");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(fd);
}

int fchdir(int fd)
{
	int ret;

	ret = next_fchdir(fd);
	if (ret) {
		perror("fchdir");
		return ret;
	}

	__verbose("%s(fd: %i)\n", __func__, fd);

	return chrootdir(NULL);
}
