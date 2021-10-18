/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <dirent.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
DIR *next___opendirat(int fd, const char *path)
{
	DIR *(*sym)(int, const char *);
	DIR *ret;

	sym = dlsym(RTLD_NEXT, "__opendirat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(fd, path);
	if (!ret)
		__perror(path, __func__);

	return ret;
}

DIR *__opendirat(int fd, const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return NULL;
	}

	__verbose("%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd, path,
		  real_path);

	return next___opendirat(fd, real_path);
}
#endif
