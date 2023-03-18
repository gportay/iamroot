/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next___open_2(const char *path, int oflags)
{
	int (*sym)(const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "__open_2");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, oflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __open_2(const char *path, int oflags)
{
	char buf[PATH_MAX];
	int atflags = 0;
	ssize_t siz;

	if (oflags & O_NOFOLLOW)
		atflags = AT_SYMLINK_NOFOLLOW;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), atflags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', oflags: 0%o)\n", __func__, path, buf,
		oflags);

	return next___open_2(buf, oflags);
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(__open_2, __open64_2);
#endif
#endif
