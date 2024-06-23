/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __NetBSD__
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

static int (*sym)(int, struct stat *);

int next___fstat50(int fd, struct stat *statbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "__fstat50");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd, statbuf);
}

int __fstat50(int fd, struct stat *statbuf)
{
	int ret;

	ret = next___fstat50(fd, statbuf);
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
