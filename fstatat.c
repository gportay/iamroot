/*
 * Copyright 2021-2023 Gaël PORTAY
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

__attribute__((visibility("hidden")))
int next_fstatat(int dfd, const char *path, struct stat *statbuf, int atflags)
{
	int (*sym)(int, const char *, struct stat *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstatat");
	if (!sym) {
		int next___fxstatat(int, int, const char *, struct stat *,
				    int);
#if defined(__arm__)
		return next___fxstatat(3, dfd, path, statbuf, atflags);
#else
		return next___fxstatat(0, dfd, path, statbuf, atflags);
#endif
	}

	ret = sym(dfd, path, statbuf, atflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int fstatat(int dfd, const char *path, struct stat *statbuf, int atflags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	uid_t uid;
	gid_t gid;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ..., atflags: 0x%x)\n",
		__func__, dfd, __fpath(dfd), path, buf, atflags);

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
	return ret;
}
