/*
 * Copyright 2021-2022 Gaël PORTAY
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
int next___openat_2(int dfd, const char *path, int flags)
{
	int (*sym)(int, const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "__openat_2");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(dfd, path, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __openat_2(int dfd, const char *path, int flags)
{
	char buf[PATH_MAX];
	int atflags = 0;
	ssize_t siz;

	if (flags & O_NOFOLLOW)
		atflags = AT_SYMLINK_NOFOLLOW;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(dfd: %d, path: '%s' -> '%s', flags: 0%o)\n", __func__, dfd,
		path, buf, flags);

	return next___openat_2(dfd, buf, flags);
}

weak_alias(__openat_2, __openat64_2);
#endif
