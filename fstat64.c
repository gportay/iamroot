/*
 * Copyright 2021-2024 Gaël PORTAY
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
extern int next___fxstat64(int, int, struct stat64 *);

static int (*sym)(int, struct stat64 *);

__attribute__((visibility("hidden")))
int next_fstat64(int fd, struct stat64 *statbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fstat64");

	if (!sym)
		return next___fxstat64(_STAT_VER, fd, statbuf);

	return sym(fd, statbuf);
}

int fstat64(int fd, struct stat64 *statbuf)
{
	uid_t uid;
	gid_t gid;
	int ret;

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
	__debug("%s(fd: %i <-> '%s', ...) -> %i\n", __func__, fd, __fpath(fd),
		ret);

	return ret;
}

int __fstat64 (int __fd, struct stat64 *__buf) __THROW __nonnull ((2));
weak_alias(fstat64, __fstat64);
#endif
#endif
