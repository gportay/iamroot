/*
 * Copyright 2022-2023 Gaël PORTAY
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
int next_getfhat(int dfd, char *path, fhandle_t *fhp, int atflags)
{
	int (*sym)(int, char *, fhandle_t *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "getfhat");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(dfd, path, fhp, atflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int getfhat(int dfd, char *path, fhandle_t *fhp, int atflags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_getfhat(dfd, buf, fhp, atflags);

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ..., atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, atflags, ret);

	return ret;
}
#endif
