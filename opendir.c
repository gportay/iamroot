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

#include <sys/types.h>
#include <dirent.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
DIR *next_opendir(const char *path)
{
	DIR *(*sym)(const char *);
	DIR *ret;

	sym = dlsym(RTLD_NEXT, "opendir");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, NULL);
	}

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

DIR *opendir(const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_opendir(buf);
}

#ifdef __GLIBC__
weak_alias(opendir, __opendir);
#endif
