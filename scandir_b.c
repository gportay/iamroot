/*
 * Copyright 2022-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <dirent.h>

#include "iamroot.h"

static int (*sym)(const char *, struct dirent ***,
		  int (*)(const struct dirent *),
		  int (*)(const struct dirent **, const struct dirent **));

hidden int next_scandir_b(const char *path, struct dirent ***namelist,
			  int (*select)(const struct dirent *),
			  int (*compar)(const struct dirent **,
				  	const struct dirent **))
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "scandir_b");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, namelist, select, compar);
}

int scandir_b(const char *path, struct dirent ***namelist,
	      int (*select)(const struct dirent *),
	      int (*compar)(const struct dirent **, const struct dirent **))
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_scandir_b(buf, namelist, select, compar);

exit:
	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}
#endif
