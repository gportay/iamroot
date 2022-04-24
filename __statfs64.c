/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#ifdef __linux__
#include <sys/statfs.h>
#endif

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next___statfs64(const char *path, struct statfs64 *statfsbuf)
{
	int (*sym)(const char *, struct statfs64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__statfs64");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, statfsbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __statfs64(const char *path, struct statfs64 *statfsbuf)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return next___statfs64(buf, statfsbuf);
}
#endif
