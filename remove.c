/*
 * Copyright 2021-2023 Gaël PORTAY
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
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int remove(const char *path)
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

	return next_remove(buf);
}
