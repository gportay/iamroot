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

#include <sys/stat.h>

#include "fpath_resolutionat.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int rootfstat(int, struct stat *);

__attribute__((visibility("hidden")))
int next_fstat(int fd, struct stat *statbuf)
{
	int (*sym)(int, struct stat *);

	sym = dlsym(RTLD_NEXT, "fstat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, statbuf);
}

int fstat(int fd, struct stat *statbuf)
{
	__fprintf(stderr, "%s(fd: %i, ...)\n", __func__, fd);

	return rootfstat(fd, statbuf);
}
