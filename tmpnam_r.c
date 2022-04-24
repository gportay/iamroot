/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

#ifndef __GLIBC_PREREQ
#define __GLIBC_PREREQ(maj,min) 0
#endif

__attribute__((visibility("hidden")))
#if defined __GLIBC__ && __GLIBC_PREREQ(2,34)
char *next_tmpnam_r(char path[L_tmpnam])
#else
char *next_tmpnam_r(char *path)
#endif
{
#if defined __GLIBC__ && !__GLIBC_PREREQ(2,34)
	char *(*sym)(char []);
#else
	char *(*sym)(char *);
#endif
	char *ret;

	sym = dlsym(RTLD_NEXT, "tmpnam_r");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

#if defined __GLIBC__ && __GLIBC_PREREQ(2,34)
char *tmpnam_r(char path[L_tmpnam])
#else
char *tmpnam_r(char *path)
#endif
{
	char buf[PATH_MAX];

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0) == -1) {
		__pathperror(path, __func__);
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_tmpnam_r(buf);
}
