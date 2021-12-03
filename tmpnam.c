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
char *next_tmpnam(char path[L_tmpnam])
#else
char *next_tmpnam(char *path)
#endif
{
#if defined __GLIBC__ && !__GLIBC_PREREQ(2,34)
	char *(*sym)(char []);
#else
	char *(*sym)(char *);
#endif
	char *ret;

	sym = dlsym(RTLD_NEXT, "tmpnam");
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
char *tmpnam(char path[L_tmpnam])
#else
char *tmpnam(char *path)
#endif
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution((char *)path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__verbose_func("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_tmpnam(real_path);
}
