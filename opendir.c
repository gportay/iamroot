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
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

DIR *opendir(const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;
	DIR *ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, NULL);

	ret = next_opendir(buf);

	__debug("%s(path: '%s' -> '%s') -> %p\n", __func__, path, buf, ret);

	return ret;
}

#ifdef __GLIBC__
weak_alias(opendir, __opendir);
#endif
