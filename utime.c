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

#include <utime.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_utime(const char *path, const struct utimbuf *times)
{
	int (*sym)(const char *, const struct utimbuf *);
	int ret;

	sym = dlsym(RTLD_NEXT, "utime");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, times);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int utime(const char *path, const struct utimbuf *times)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_utime(buf, times);
}
