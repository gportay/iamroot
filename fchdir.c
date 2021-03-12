/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern char *next_getcwd(char *, size_t);
extern int chrootdir(const char *);

__attribute__((visibility("hidden")))
int next_fchdir(int fd)
{
	int (*sym)(int);

	sym = dlsym(RTLD_NEXT, "fchdir");
	if (!sym) {
		errno = ENOTSUP;
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

	__fprintf(stderr, "%s(fd: %i)\n", __func__, fd);

	return chrootdir(NULL);
}
