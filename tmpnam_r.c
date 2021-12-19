/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <limits.h>
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
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path);
	if (!ret)
		__perror(path, __func__);

	return ret;
}

#if defined __GLIBC__ && __GLIBC_PREREQ(2,34)
char *tmpnam_r(char path[L_tmpnam])
#else
char *tmpnam_r(char *path)
#endif
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__perror(path, "path_resolution");
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_tmpnam_r(real_path);
}
