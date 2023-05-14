/*
 * Copyright 2021-2023 Gaël PORTAY
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
int next_unlinkat(int dfd, const char *path, int atflags)
{
	int (*sym)(int, const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "unlinkat");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(dfd, path, atflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int unlinkat(int dfd, const char *path, int atflags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf),
			      atflags | AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_unlinkat(dfd, buf, atflags);

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, atflags, ret);

	return ret;
}
