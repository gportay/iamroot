/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <dirent.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
DIR *next___opendir2(const char *path, int flags)
{
	DIR *(*sym)(const char *, int);
	DIR *ret;

	sym = dlsym(RTLD_NEXT, "__opendir2");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path, flags);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

DIR *__opendir2(const char *path, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s', flags: 0x%x)\n", __func__, path, buf,
		flags);

	return next___opendir2(buf, flags);
}
#endif
