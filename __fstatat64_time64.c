/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/xattr.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

#ifdef __GLIBC__
#ifdef _LARGEFILE64_SOURCE
#if __TIMESIZE == 32
static int (*sym)(int, const char *, struct stat64 *, int);

hidden int next___fstatat64_time64(int dfd, const char *path,
				   struct stat64 *statbuf, int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "__fstatat64_time64");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, statbuf, atflags);
}

int __fstatat64_time64(int dfd, const char *path, struct stat64 *statbuf,
		       int atflags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

	ret = next___fstatat64_time64(dfd, buf, statbuf, atflags);
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
#endif
#endif
#endif
