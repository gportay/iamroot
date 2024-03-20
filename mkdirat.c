/*
 * Copyright 2021-2024 Gaël PORTAY
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

hidden int next_mkdirat(int dfd, const char *path, mode_t mode)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "mkdirat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, mode);
}

int mkdirat(int dfd, const char *path, mode_t mode)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	(void)oldmode;

	siz = path_resolution(dfd, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	__fwarn_if_insuffisant_user_modeat(dfd, buf, mode, 0);

	ret = next_mkdirat(dfd, buf, mode);
	__set_mode(buf, oldmode, mode);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', mode: 0%03o -> 0%03o) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, oldmode, mode, ret);

	return ret;
}
