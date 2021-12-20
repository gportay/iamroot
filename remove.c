/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_remove(const char *path)
{
	int (*sym)(const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "remove");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int remove(const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_remove(real_path);
}
