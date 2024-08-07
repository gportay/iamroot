/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined __linux__ || defined __FreeBSD__
#include <unistd.h>
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

static int (*sym)(int, int, const char *, struct stat *, int);

hidden int next___fxstatat(int ver, int dfd, const char *path,
			   struct stat *statbuf, int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "__fxstatat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(ver, dfd, path, statbuf, atflags);
}

int __fxstatat(int ver, int dfd, const char *path, struct stat *statbuf,
	       int atflags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

	ret = next___fxstatat(ver, dfd, buf, statbuf, atflags);
	if (ret == -1)
		goto exit;

	__st_mode(buf, statbuf);
	__st_uid(buf, statbuf);
	__st_gid(buf, statbuf);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ..., atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, atflags, ret);

	return ret;
}
#endif
