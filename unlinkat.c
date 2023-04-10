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
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(dfd, path, atflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int unlinkat(int dfd, const char *path, int atflags)
{
	char buf[PATH_MAX];

	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf),
			      atflags | AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', atflags: 0x%x)\n",
		__func__, dfd, __fpath(dfd), path, buf, atflags);

	__remove_at_empty_path_if_needed(buf, atflags);
	return next_unlinkat(dfd, buf, atflags);
}
