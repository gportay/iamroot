/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/xattr.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

#ifdef _LARGEFILE64_SOURCE
__attribute__((visibility("hidden")))
int next_fstat64(int fd, struct stat64 *statbuf)
{
	int (*sym)(int, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstat64");
	if (!sym) {
		int next___fxstat64(int, int, struct stat64 *);
#if defined(__arm__)
		return next___fxstat64(3, fd, statbuf);
#else
		return next___fxstat64(0, fd, statbuf);
#endif
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fstat64(int fd, struct stat64 *statbuf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	__debug("%s(fd: %i <-> '%s', ...)\n", __func__, fd, __fpath(fd));

	ret = next_fstat64(fd, statbuf);
	if (ret == -1)
		goto exit;

	uid = __fget_uid(fd);
	if (uid == (uid_t)-1)
		statbuf->st_uid = 0;

	gid = __fget_gid(fd);
	if (gid == (gid_t)-1)
		statbuf->st_gid = 0;

	__fst_mode(fd, statbuf);
	__fst_uid(fd, statbuf);
	__fst_gid(fd, statbuf);

exit:
	return ret;
}

weak_alias(fstat64, __fstat64);
weak_alias(fstat64, __fstat_time64);
#endif
#endif
