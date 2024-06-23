/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined __linux__ || defined __FreeBSD__
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

static int (*sym)(int, int, struct stat *);

int next___fxstat(int ver, int fd, struct stat *statbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "__fxstat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(ver, fd, statbuf);
}

int __fxstat(int ver, int fd, struct stat *statbuf)
{
	int ret;

	ret = next___fxstat(ver, fd, statbuf);
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
