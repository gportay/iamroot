/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_mkfifoat(int dfd, const char *path, mode_t mode)
{
	int (*sym)(int, const char *, mode_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "mkfifoat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(dfd, path, mode);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int mkfifoat(int dfd, const char *path, mode_t mode)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__fwarn_if_insuffisant_user_modeat(dfd, buf, mode, 0);
	__debug("%s(dfd: %d, path: '%s' -> '%s', mode: 0%03o -> 0%03o)\n",
		__func__, dfd, path, buf, oldmode, mode);

	ret = next_mkfifoat(dfd, buf, mode);
#ifdef __linux__
	__set_mode(buf, oldmode, mode);
#endif

	return ret;
}
