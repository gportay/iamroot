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
int next_unlinkat(int fd, const char *path, int flags)
{
	int (*sym)(int, const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "unlinkat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int unlinkat(int fd, const char *path, int flags)
{
	char buf[PATH_MAX];

	if (path_resolution(fd, path, buf, sizeof(buf),
			    flags | AT_SYMLINK_NOFOLLOW) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %d, path: '%s' -> '%s', flags: 0x%x)\n", __func__, fd,
		path, buf, flags);

	return next_unlinkat(fd, buf, flags);
}
