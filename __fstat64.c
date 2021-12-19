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
int next___fstat64(int fd, struct stat64 *statbuf)
{
	int (*sym)(int, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__fstat64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fperror(fd, __func__);

	return ret;
}

int __fstat64(int fd, struct stat64 *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;
	ssize_t siz;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		perror("__procfdreadlink");
		return -1;
	}
	buf[siz] = 0;
	real_path = buf;

	__debug("%s(fd: %i <-> '%s', ...)\n", __func__, fd, real_path);

	return __rootfstat64(fd, statbuf);
}
#endif
