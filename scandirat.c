/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <dirent.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_scandirat(int fd, const char *path, struct dirent ***namelist,
		  int (*filter)(const struct dirent *),
		  int (*compar)(const struct dirent **, const struct dirent **))
{
	int (*sym)(int, const char *, struct dirent ***,
		   int (*)(const struct dirent *),
		   int (*)(const struct dirent **, const struct dirent **));
	int ret;

	sym = dlsym(RTLD_NEXT, "scandirat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, namelist, filter, compar);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int scandirat(int fd, const char *path, struct dirent ***namelist,
	      int (*filter)(const struct dirent *),
	      int (*compar)(const struct dirent **, const struct dirent **))
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__perror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_scandirat(fd, real_path, namelist, filter, compar);
}
