/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_open(const char *path, int flags, mode_t mode)
{
	int (*sym)(const char *, int, ...);
	int ret;

	sym = dlsym(RTLD_NEXT, "open");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, flags, mode);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int open(const char *path, int flags, ...)
{
	char buf[PATH_MAX];
	int atflags = 0;
	mode_t mode = 0;

	if (flags & O_NOFOLLOW)
		atflags = AT_SYMLINK_NOFOLLOW;

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf), atflags) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

#ifdef __linux__
	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
#endif

	__debug("%s(path: '%s' -> '%s', flags: 0%o, mode: 0%03o)\n", __func__,
		path, buf, flags, mode);
	if (flags & O_CREAT)
		__warn_if_insuffisant_user_mode(buf, mode);

	return next_open(buf, flags, mode);
}
