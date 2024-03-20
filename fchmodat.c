/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif
#if defined __FreeBSD__ || defined __NetBSD__
#include <sys/extattr.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

static int (*sym)(int, const char *, mode_t, int);

hidden int next_fchmodat(int dfd, const char *path, mode_t mode, int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fchmodat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, mode, atflags);
}

int fchmodat(int dfd, const char *path, mode_t mode, int atflags)
{
	const int errno_save = errno;
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	(void)oldmode;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

	__fwarn_if_insuffisant_user_modeat(dfd, buf, mode, atflags);

	ret = next_fchmodat(dfd, buf, mode, atflags);
	__ignore_error_and_warn(ret, dfd, path, atflags);
	/* Force ignoring EPERM error if not chroot'ed */
	if ((ret == -1) && (errno == EPERM))
		ret = __set_errno(errno_save, 0);
	__set_mode(buf, oldmode, mode);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', mode: 0%03o -> 0%03o, atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, oldmode, mode,
		atflags, ret);

	return ret;
}
