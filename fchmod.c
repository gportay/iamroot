/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif
#ifdef __FreeBSD__
#include <sys/extattr.h>
#endif

#include <sys/stat.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fchmod(int fd, mode_t mode)
{
	int (*sym)(int, mode_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchmod");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(fd, mode);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fchmod(int fd, mode_t mode)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return __fpath_perror(fd, -1);

	__debug("%s(fd: %i <-> '%s', mode: 0%03o -> 0%03o)\n", __func__, fd,
		__fpath(fd), oldmode, mode);
	__fwarn_if_insuffisant_user_mode(fd, mode);

	ret = next_fchmod(fd, mode);
	__ignore_error_and_warn(ret, AT_FDCWD, buf, 0);
	/* Force ignoring EPERM error if not chroot'ed */
	if ((ret == -1) && (errno == EPERM))
		ret = __set_errno(0, 0);
	__set_mode(buf, oldmode, mode);

	return ret;
}
