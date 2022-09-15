/*
 * Copyright 2021-2022 Gaël PORTAY
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
extern uid_t next_getegid();
extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next___fxstatat64(int ver, int dfd, const char *path,
		      struct stat64 *statbuf, int atflags)
{
	int (*sym)(int, int, const char *, struct stat64 *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "__fxstatat64");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(ver, dfd, path, statbuf, atflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __fxstatat64(int ver, int dfd, const char *path, struct stat64 *statbuf,
		 int atflags)
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
	ret = next___fxstatat64(ver, dfd, buf, statbuf, atflags);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (statbuf->st_uid == uid)
		statbuf->st_uid = geteuid();

	gid = next_getegid();
	if (statbuf->st_gid == gid)
		statbuf->st_gid = 0;

	__st_mode(buf, statbuf);

exit:
	return ret;
}
#endif
#endif
