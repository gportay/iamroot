/*
 * Copyright 2021-2022 Gaël PORTAY
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
int next___open64_2(const char *path, int flags)
{
	int (*sym)(const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "__open64_2");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __open64_2(const char *path, int flags)
{
	char buf[PATH_MAX];
	int atflags = 0;

	if (flags & O_NOFOLLOW)
		atflags = AT_SYMLINK_NOFOLLOW;

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf), atflags) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', flags: 0%o)\n", __func__, path, buf,
		flags);

	return next___open64_2(buf, flags);
}
#endif
