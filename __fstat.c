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

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int __rootfstat(int, struct stat *);

__attribute__((visibility("hidden")))
int next___fstat(int fd, struct stat *statbuf)
{
	int (*sym)(int, struct stat *);

	sym = dlsym(RTLD_NEXT, "__fstat");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, statbuf);
}

int __fstat(int fd, struct stat *statbuf)
{
	__fprintf(stderr, "%s(fd: %i, ...)\n", __func__, fd);

	return __rootfstat(fd, statbuf);
}
