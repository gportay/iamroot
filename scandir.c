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

#include <dirent.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_scandir(const char *path, struct dirent ***namelist,
		 int (*filter)(const struct dirent *),
		 int (*compar)(const struct dirent **, const struct dirent **))
{
	int (*sym)(const char *, struct dirent ***,
		   int (*)(const struct dirent *),
		   int (*)(const struct dirent **, const struct dirent **));
	int ret;

	sym = dlsym(RTLD_NEXT, "scandir");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path, namelist, filter, compar);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int scandir(const char *path, struct dirent ***namelist,
	    int (*filter)(const struct dirent *),
	    int (*compar)(const struct dirent **, const struct dirent **))
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_scandir(buf, namelist, filter, compar);
}
