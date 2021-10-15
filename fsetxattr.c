/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/xattr.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fsetxattr(int fd, const char *name, const void *value, size_t size,
		   int flags)
{
	int (*sym)(int, const char *, const void *, size_t, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fsetxattr");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, name, value, size, flags);
	if (ret == -1)
		__fperror(fd, __func__);

	return ret;
}

int fsetxattr(int fd, const char *name, const void *value, size_t size,
	      int flags)
{
	__verbose("%s(fd: %i, name: '%s', ...)\n", __func__, fd, name);

	return next_fsetxattr(fd, name, value, size, flags);
}
