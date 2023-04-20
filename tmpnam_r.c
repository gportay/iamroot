/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
#if __GLIBC_PREREQ(2,34)
char *next_tmpnam_r(char path[L_tmpnam])
#else
char *next_tmpnam_r(char *path)
#endif
{
#if __GLIBC_PREREQ(2,34)
	char *(*sym)(char []);
#else
	char *(*sym)(char *);
#endif
	char *ret;

	sym = dlsym(RTLD_NEXT, "tmpnam_r");
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

#if __GLIBC_PREREQ(2,34)
char *tmpnam_r(char path[L_tmpnam])
#else
char *tmpnam_r(char *path)
#endif
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, NULL);

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_tmpnam_r(buf);
}
#endif
