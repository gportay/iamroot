/*
 * Copyright 2021-2023 Gaël PORTAY
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
int next_lchmod(const char *path, mode_t mode)
{
	int (*sym)(const char *, mode_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "lchmod");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path, mode);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int lchmod(const char *path, mode_t mode)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;
	(void)oldmode;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	__warn_if_insuffisant_user_mode(buf, mode);
	__debug("%s(path: '%s' -> '%s', mode: 0%03o -> 0%03o)\n", __func__,
		path, buf, oldmode, mode);

	ret = next_lchmod(buf, mode);
	__ignore_error_and_warn(ret, AT_FDCWD, path, 0);

	return ret;
}
