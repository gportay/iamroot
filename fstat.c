/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
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

int next_fstat(int fd, struct stat *statbuf)
{
	int (*sym)(int, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstat");
	if (!sym) {
		int next___fxstat(int, int, struct stat *);
#if defined(__arm__)
		return next___fxstat(3, fd, statbuf);
#else
		return next___fxstat(0, fd, statbuf);
#endif
	}

	ret = sym(fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fstat(int fd, struct stat *statbuf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_fstat(fd, statbuf);
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

#ifdef __GLIBC__
weak_alias(fstat, __fstat);
#endif
