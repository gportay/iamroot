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
#if defined __FreeBSD__ || defined __NetBSD__
#include <sys/extattr.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>

#include "iamroot.h"

static int (*sym)(const char *, mode_t);

hidden int next_creat(const char *path, mode_t mode)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "creat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, mode);
}

int creat(const char *path, mode_t mode)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	(void)oldmode;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	__warn_if_insuffisant_user_mode(buf, mode);

	ret = next_creat(buf, mode);
	__set_mode(buf, oldmode, mode);

	if (ret >= 0)
		__info("%s: %i -> '%s'\n", __func__, ret, __fpath(ret));

exit:
	__debug("%s(path: '%s' -> '%s', mode: 0%03o -> 0%03o) -> %i\n",
		__func__, path, buf, oldmode, mode, ret);

	return ret;
}

#ifdef __GLIBC__
weak_alias(creat, creat64);
#endif
