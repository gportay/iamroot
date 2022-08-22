/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

extern uid_t next_geteuid();

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
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(dfd: %i, path: '%s' -> '%s', ..., atflags: 0x%x)\n",
		__func__, dfd, path, buf, atflags);

	__remove_at_empty_path_if_needed(buf, atflags);
	ret = next_fstatat(dfd, buf, statbuf, atflags);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (statbuf->st_uid == uid)
		statbuf->st_uid = geteuid();

	gid = getegid();
	if (statbuf->st_gid == gid)
		statbuf->st_gid = 0;

exit:
	return ret;
}
