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
#include <sys/stat.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fchmodat(int fd, const char *path, mode_t mode, int flags)
{
	int (*sym)(int, const char *, mode_t, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchmodat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, mode, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int fchmodat(int fd, const char *path, mode_t mode, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(fd, path, buf, sizeof(buf), flags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', mode: 0%03o, flags: 0x%x)\n",
		__func__, fd, path, buf, mode, flags);
	__fwarn_if_insuffisant_user_modeat(fd, buf, mode, flags);

	__remove_at_empty_path_if_needed(buf, flags);
	ret = next_fchmodat(fd, buf, mode, flags);
	__ignore_error_and_warn(ret, fd, path, flags);

	return ret;
}
