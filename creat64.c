/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/stat.h>
#include <fcntl.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next_creat64(const char *path, mode_t mode)
{
	int (*sym)(const char *, mode_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "creat64");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, mode);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int creat64(const char *path, mode_t mode)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', mode: 0%03o)\n", __func__, path,
		real_path, mode);
	__warn_if_insuffisant_user_mode(real_path, mode);

	return next_creat64(real_path, mode);
}
#endif
