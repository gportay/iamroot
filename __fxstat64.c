/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/stat.h>

#include "iamroot.h"

#ifdef __GLIBC__
extern int __rootfxstat64(int, int, struct stat64 *);

__attribute__((visibility("hidden")))
int next___fxstat64(int ver, int fd, struct stat64 *stat64buf)
{
	int (*sym)(int, int, struct stat64 *);

	sym = dlsym(RTLD_NEXT, "__fxstat64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(ver, fd, stat64buf);
}

int __fxstat64(int ver, int fd, struct stat64 *stat64buf)
{
	__verbose("%s(fd: %i, ...)\n", __func__, fd);

	return __rootfxstat64(ver, fd, stat64buf);
}
#endif
