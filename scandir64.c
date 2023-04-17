/*
 * Copyright 2022-2023 Gaël PORTAY
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

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next_scandir64(const char *path, struct dirent64 ***namelist,
		   int (*filter)(const struct dirent64 *),
		   int (*compar)(const struct dirent64 **,
				 const struct dirent64 **))
{
	int (*sym)(const char *, struct dirent64 ***,
		   int (*)(const struct dirent64 *),
		   int (*)(const struct dirent64 **,
			   const struct dirent64 **));
	int ret;

	sym = dlsym(RTLD_NEXT, "scandir64");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path, namelist, filter, compar);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int scandir64(const char *path, struct dirent64 ***namelist,
	      int (*filter)(const struct dirent64 *),
	      int (*compar)(const struct dirent64 **,
			    const struct dirent64 **))
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_scandir64(buf, namelist, filter, compar);
}
#endif
