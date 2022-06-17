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
#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_faccessat(int fd, const char *path, int mode, int flags)
{
	int (*sym)(int, const char *, int, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "faccessat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, mode, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int faccessat(int fd, const char *path, int mode, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(fd, path, buf, sizeof(buf), flags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', flags: 0x%x)\n", __func__, fd,
		path, buf, flags);

	__remove_at_empty_path_if_needed(buf, flags);
	return next_faccessat(fd, buf, mode, flags);
}
