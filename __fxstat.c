/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/xattr.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

int next___fxstat(int ver, int fd, struct stat *statbuf)
{
	int (*sym)(int, int, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__fxstat");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(ver, fd, statbuf);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int __fxstat(int ver, int fd, struct stat *statbuf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	__debug("%s(fd: %i <-> '%s', ...)\n", __func__, fd, __fpath(fd));

	ret = next___fxstat(ver, fd, statbuf);
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
