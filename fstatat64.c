/*
 * Copyright 2021-2024 Gaël PORTAY
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

#ifdef _LARGEFILE64_SOURCE
extern int next___fxstatat64(int, int, const char *, struct stat64 *, int);
static int (*sym)(int, const char *, struct stat64 *, int);

__attribute__((visibility("hidden")))
int next_fstatat64(int dfd, const char *path, struct stat64 *statbuf,
		   int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fstatat64");

	if (!sym)
		return next___fxstatat64(_STAT_VER, dfd, path, statbuf,
					 atflags);

	return sym(dfd, path, statbuf, atflags);
}

int fstatat64(int dfd, const char *path, struct stat64 *statbuf, int atflags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	uid_t uid;
	gid_t gid;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

	ret = next_fstatat64(dfd, buf, statbuf, atflags);
	if (ret == -1)
		goto exit;

	uid = __get_uid(buf);
	if (uid == (uid_t)-1)
		statbuf->st_uid = 0;

	gid = __get_gid(buf);
	if (gid == (gid_t)-1)
		statbuf->st_gid = 0;

	__st_mode(buf, statbuf);
	__st_uid(buf, statbuf);
	__st_gid(buf, statbuf);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ..., atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, atflags, ret);

	return ret;
}

int __fstatat64 (int __fd, const char *__restrict __file,
		 struct stat64 *__restrict __buf, int __flag)
     __THROW __nonnull ((2, 3));
weak_alias(fstatat64, __fstatat64);
#endif
#endif
