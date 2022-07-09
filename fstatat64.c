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

#ifdef __GLIBC__
extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next_fstatat64(int fd, const char *path, struct stat64 *statbuf, int flags)
{
	int (*sym)(int, const char *, struct stat64 *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstatat64");
	if (!sym) {
		int next___fxstatat64(int, int, const char *, struct stat64 *,
				      int);
#if defined(__arm__)
		return next___fxstatat64(3, fd, path, statbuf, flags);
#else
		return next___fxstatat64(0, fd, path, statbuf, flags);
#endif
	}

	ret = sym(fd, path, statbuf, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int fstatat64(int fd, const char *path, struct stat64 *statbuf, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	uid_t uid;
	gid_t gid;
	int ret;

	siz = path_resolution(fd, path, buf, sizeof(buf), flags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', ..., flags: 0x%x)\n", __func__,
		fd, path, buf, flags);

	__remove_at_empty_path_if_needed(buf, flags);
	ret = next_fstatat64(fd, buf, statbuf, flags);
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

weak_alias(fstatat64, __fstatat64);
weak_alias(fstatat64, __fstatat_time64);
#endif
