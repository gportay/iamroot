/*
 * Copyright 2022-2023 Gaël PORTAY
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
DIR *next___opendir2(const char *path, int oflags)
{
	DIR *(*sym)(const char *, int);
	DIR *ret;

	sym = dlsym(RTLD_NEXT, "__opendir2");
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(path, oflags);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

DIR *__opendir2(const char *path, int oflags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s', oflags: 0%o)\n", __func__, path, buf,
		oflags);

	return next___opendir2(buf, oflags);
}
#endif
