/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/xattr.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

#ifdef _LARGEFILE64_SOURCE
static int (*sym)(int, int, struct stat64 *);

int next___fxstat64(int ver, int fd, struct stat64 *statbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "__fxstat64");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(ver, fd, statbuf);
}

int __fxstat64(int ver, int fd, struct stat64 *statbuf)
{
	int ret;

	ret = next___fxstat64(ver, fd, statbuf);
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
#endif
#endif
