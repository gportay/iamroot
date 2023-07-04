/*
 * Copyright 2021-2023 Gaël PORTAY
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

static int (*sym)(int, const char *, struct dirent ***,
		  int (*)(const struct dirent *),
		  int (*)(const struct dirent **, const struct dirent **));

__attribute__((visibility("hidden")))
int next_scandirat(int dfd, const char *path, struct dirent ***namelist,
		  int (*filter)(const struct dirent *),
		  int (*compar)(const struct dirent **, const struct dirent **))
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "scandirat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, namelist, filter, compar);
}

int scandirat(int dfd, const char *path, struct dirent ***namelist,
	      int (*filter)(const struct dirent *),
	      int (*compar)(const struct dirent **, const struct dirent **))
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_scandirat(dfd, buf, namelist, filter, compar);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ...) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, ret);

	return ret;
}
