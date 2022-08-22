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
int next_faccessat(int dfd, const char *path, int mode, int atflags)
{
	int (*sym)(int, const char *, int, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "faccessat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(dfd, path, mode, atflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int faccessat(int dfd, const char *path, int mode, int atflags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(dfd: %i, path: '%s' -> '%s', mode: 0%03o, atflags: 0x%x)\n",
		__func__, dfd, path, buf, mode, atflags);

	__remove_at_empty_path_if_needed(buf, atflags);
	return next_faccessat(dfd, buf, mode, atflags);
}
