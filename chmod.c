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

#include <sys/stat.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_chmod(const char *path, mode_t mode)
{
	int (*sym)(const char *, mode_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "chmod");
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

int chmod(const char *path, mode_t mode)
{
	char buf[PATH_MAX];
	char *real_path;
	int ret;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', mode: 0%03o)\n", __func__, path,
		real_path, mode);
	__warn_if_insuffisant_user_mode(real_path, mode);

	ret = next_chmod(real_path, mode);
	__ignore_error_and_warn(ret, AT_FDCWD, path, 0);

	return ret;
}
