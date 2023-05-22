/*
 * Copyright 2021-2023 Gaël PORTAY
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
#ifdef __FreeBSD__
#include <sys/extattr.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

static int (*sym)(int, const char *, mode_t);

__attribute__((visibility("hidden")))
int next_mkfifoat(int dfd, const char *path, mode_t mode)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "mkfifoat");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(dfd, path, mode);
}

int mkfifoat(int dfd, const char *path, mode_t mode)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;
	(void)oldmode;

	siz = path_resolution(dfd, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	__fwarn_if_insuffisant_user_modeat(dfd, buf, mode, 0);

	ret = next_mkfifoat(dfd, buf, mode);
	__set_mode(buf, oldmode, mode);

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', mode: 0%03o -> 0%03o) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, oldmode, mode, ret);

	return ret;
}
