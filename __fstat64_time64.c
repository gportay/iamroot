/*
 * Copyright 2023-2024 Gaël PORTAY
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

#ifdef __GLIBC__
#ifdef _LARGEFILE64_SOURCE
#if __TIMESIZE == 32
static int (*sym)(int, struct stat64 *);

hidden int next___fstat64_time64(int fd, struct stat64 *statbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "__fstat64_time64");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd, statbuf);
}

int __fstat64_time64(int fd, struct stat64 *statbuf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___fstat64_time64(fd, statbuf);
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
#endif
#endif
#endif
#endif
