/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/param.h>
#include <sys/mount.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_getfhat(int fd, char *path, fhandle_t *fhp, int flags)
{
	int (*sym)(int, char *, fhandle_t *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "getfhat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, fhp, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int getfhat(int fd, char *path, fhandle_t *fhp, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(fd, path, buf, sizeof(buf), flags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', ..., flags: 0x%x)\n", __func__,
		fd, path, buf, flags);

	return next_getfhat(fd, buf, fhp, flags);
}
#endif
