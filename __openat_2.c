/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next___openat_2(int dfd, const char *path, int oflags)
{
	int (*sym)(int, const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "__openat_2");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(dfd, path, oflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __openat_2(int dfd, const char *path, int oflags)
{
	char buf[PATH_MAX];
	int atflags = 0;
	ssize_t siz;
	int ret;

	if (oflags & O_NOFOLLOW)
		atflags = AT_SYMLINK_NOFOLLOW;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next___openat_2(dfd, buf, oflags);
	if (ret >= 0)
		__setfd(ret, buf);

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', oflags: 0%o) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, oflags, ret);

	return ret;
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(__openat_2, __openat64_2);
#endif
#endif
