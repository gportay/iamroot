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
int next_chmod(const char *path, mode_t mode)
{
	int (*sym)(const char *, mode_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "chmod");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path, mode);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int chmod(const char *path, mode_t mode)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	__warn_if_insuffisant_user_mode(buf, mode);
	__debug("%s(path: '%s' -> '%s', mode: 0%03o -> 0%03o)\n", __func__,
		path, buf, oldmode, mode);

	ret = next_chmod(buf, mode);
	__ignore_error_and_warn(ret, AT_FDCWD, path, 0);
	/* Force ignoring EPERM error if not chroot'ed */
	if ((ret == -1) && (errno == EPERM))
		ret = __set_errno(0, 0);
	__set_mode(buf, oldmode, mode);

	return ret;
}
