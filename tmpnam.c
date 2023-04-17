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
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(path);
	if (!ret)
		__pathperror(path, __func__);

	return ret;
}

#if defined __GLIBC__ && __GLIBC_PREREQ(2,34)
char *tmpnam(char path[L_tmpnam])
#else
char *tmpnam(char *path)
#endif
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, (char *)path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_tmpnam(buf);
}
