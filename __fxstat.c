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

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int __rootfxstat(int, int, struct stat *);

__attribute__((visibility("hidden")))
int next___fxstat(int ver, int fd, struct stat *statbuf)
{
	int (*sym)(int, int, struct stat *);

	sym = dlsym(RTLD_NEXT, "__fxstat");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(ver, fd, statbuf);
}

int __fxstat(int ver, int fd, struct stat *statbuf)
{
	__fprintf(stderr, "%s(fd: %i, ...)\n", __func__, fd);

	return __rootfxstat(ver, fd, statbuf);
}
