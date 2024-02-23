/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/stat.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif
#ifdef __FreeBSD__
#include <sys/extattr.h>
#endif

#include <fcntl.h>

#include "iamroot.h"

static int (*sym)(int, const char *, int, ...);

__attribute__((visibility("hidden")))
int next_openat(int dfd, const char *path, int oflags, mode_t mode)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "openat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, oflags, mode);
}

int openat(int dfd, const char *path, int oflags, ...)
{
	mode_t oldmode = 0, mode = 0;
	int atflags = 0, ret = -1;
	char buf[PATH_MAX];
	ssize_t siz;
	(void)oldmode;

	if (oflags & O_NOFOLLOW)
		atflags = AT_SYMLINK_NOFOLLOW;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

#ifdef __linux__
	if (__needs_mode(oflags)) {
		va_list ap;
		va_start(ap, oflags);
		mode = va_arg(ap, mode_t);
		oldmode = mode;
		va_end(ap);
	}
#endif

	if (oflags & O_CREAT)
		__fwarn_if_insuffisant_user_modeat(dfd, buf, mode, 0);

	ret = next_openat(dfd, buf, oflags, mode);
	__set_mode(buf, oldmode, mode);
	if (ret >= 0)
		__setfd(ret, buf);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', oflags: 0%o, mode: 0%03o -> 0%03o) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, oflags, oldmode, mode,
		ret);

	return ret;
}

#ifdef _LARGEFILE64_SOURCE
weak_alias(openat, openat64);
#endif
