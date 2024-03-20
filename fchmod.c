/*
 * Copyright 2021-2024 Gaël PORTAY
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
#if defined __FreeBSD__ || defined __NetBSD__
#include <sys/extattr.h>
#endif

#include <sys/stat.h>

#include "iamroot.h"

static int (*sym)(int, mode_t);

hidden int next_fchmod(int fd, mode_t mode)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fchmod");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd, mode);
}

int fchmod(int fd, mode_t mode)
{
	const int errno_save = errno;
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	(void)oldmode;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		goto exit;

	__fwarn_if_insuffisant_user_mode(fd, mode);

	ret = next_fchmod(fd, mode);
	__ignore_error_and_warn(ret, AT_FDCWD, buf, 0);
	/* Force ignoring EPERM error if not chroot'ed */
	if ((ret == -1) && (errno == EPERM))
		ret = __set_errno(errno_save, 0);
	__set_mode(buf, oldmode, mode);

exit:
	__debug("%s(fd: %i <-> '%s', mode: 0%03o -> 0%03o) -> %i\n", __func__,
		fd, __fpath(fd), oldmode, mode, ret);

	return ret;
}
