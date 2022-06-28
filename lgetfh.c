/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/param.h>
#include <sys/mount.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_lgetfh(const char *path, fhandle_t *fhp)
{
	int (*sym)(const char *, fhandle_t *);
	int ret;

	sym = dlsym(RTLD_NEXT, "lgetfh");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, fhp);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int lgetfh(const char *path, fhandle_t *fhp)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return next_lgetfh(path, fhp);
}
#endif
