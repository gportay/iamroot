/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

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

#ifdef __linux__
extern int next___fxstatat(int, int, const char *, struct stat *, int);
#endif

static int (*sym)(int, const char *, struct stat *, int);

hidden
int next_fstatat(int dfd, const char *path, struct stat *statbuf, int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fstatat");

#ifdef __linux__
	if (!sym)
		return next___fxstatat(_STAT_VER, dfd, path, statbuf, atflags);
#else
	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);
#endif

	return sym(dfd, path, statbuf, atflags);
}

int fstatat(int dfd, const char *path, struct stat *statbuf, int atflags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	uid_t uid;
	gid_t gid;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

	ret = next_fstatat(dfd, buf, statbuf, atflags);
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
