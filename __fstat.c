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

extern int __rootfstat(int, struct stat *);

__attribute__((visibility("hidden")))
int next___fstat(int fd, struct stat *statbuf)
{
	int (*sym)(int, struct stat *);

	sym = dlsym(RTLD_NEXT, "__fstat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, statbuf);
}

int __fstat(int fd, struct stat *statbuf)
{
	__verbose("%s(fd: %i, ...)\n", __func__, fd);

	return __rootfstat(fd, statbuf);
}
