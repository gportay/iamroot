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
int next_fchmodat(int dfd, const char *path, mode_t mode, int atflags)
{
	int (*sym)(int, const char *, mode_t, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchmodat");
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

int fchmodat(int dfd, const char *path, mode_t mode, int atflags)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__fwarn_if_insuffisant_user_modeat(dfd, buf, mode, atflags);
	__debug("%s(dfd: %i, path: '%s' -> '%s', mode: 0%03o -> 0%03o, atflags: 0x%x)\n",
		__func__, dfd, path, buf, oldmode, mode, atflags);

	__remove_at_empty_path_if_needed(buf, atflags);
	ret = next_fchmodat(dfd, buf, mode, atflags);
	__ignore_error_and_warn(ret, dfd, path, atflags);
#ifdef __linux__
	__set_mode(buf, oldmode, mode);
#endif

	return ret;
}
