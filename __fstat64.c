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

#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int __rootfstat64(int, struct stat64 *);

__attribute__((visibility("hidden")))
int next___fstat64(int fd, struct stat64 *stat64buf)
{
	int (*sym)(int, struct stat64 *);

	sym = dlsym(RTLD_NEXT, "__fstat64");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, stat64buf);
}

int __fstat64(int fd, struct stat64 *stat64buf)
{
	__fprintf(stderr, "%s(fd: %i, ...)\n", __func__, fd);

	return __rootfstat64(fd, stat64buf);
}
#endif
