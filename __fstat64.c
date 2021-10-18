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

#ifdef __GLIBC__
extern int __rootfstat64(int, struct stat64 *);

__attribute__((visibility("hidden")))
int next___fstat64(int fd, struct stat64 *stat64buf)
{
	int (*sym)(int, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__fstat64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, stat64buf);
	if (ret == -1)
		__fperror(fd, __func__);

	return ret;
}

int __fstat64(int fd, struct stat64 *stat64buf)
{
	__verbose("%s(fd: %i, ...)\n", __func__, fd);

	return __rootfstat64(fd, stat64buf);
}
#endif
