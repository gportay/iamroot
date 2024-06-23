/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __NetBSD__
#define __LIBC12_SOURCE__
#endif

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

#ifdef __linux__
extern int next___fxstat(int, int, struct stat *);
#endif

static int (*sym)(int, struct stat *);

int next_fstat(int fd, struct stat *statbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fstat");

#ifdef __linux__
	if (!sym)
		return next___fxstat(_STAT_VER, fd, statbuf);
#else
	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);
#endif

	return sym(fd, statbuf);
}

int fstat(int fd, struct stat *statbuf)
{
	int ret;

	ret = next_fstat(fd, statbuf);
	if (ret == -1)
		goto exit;

	__fst_mode(fd, statbuf);
	__fst_uid(fd, statbuf);
	__fst_gid(fd, statbuf);

exit:
	__debug("%s(fd: %i <-> '%s', ...) -> %i\n", __func__, fd, __fpath(fd),
		ret);

	return ret;
}

#ifdef __GLIBC__
int __fstat (int __fd, struct stat *__buf) __THROW __nonnull ((2));
weak_alias(fstat, __fstat);
#endif
