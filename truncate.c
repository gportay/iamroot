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

#include <unistd.h>
#include <sys/types.h>

int truncate(const char *path, off_t length);

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_truncate(const char *path, off_t length)
{
	int (*sym)(const char *, off_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "truncate");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, length);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int truncate(const char *path, off_t length)
{
	char buf[PATH_MAX];

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return next_truncate(buf, length);
}
