/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __NetBSD__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <dirent.h>

#include "iamroot.h"

static DIR *(*sym)(const char *);

hidden DIR *next_opendir(const char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "opendir");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(path);
}

DIR *opendir(const char *path)
{
	char buf[PATH_MAX];
	DIR *ret = NULL;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_opendir(buf);

exit:
	__debug("%s(path: '%s' -> '%s') -> %p\n", __func__, path, buf, ret);

	return ret;
}

#ifdef __GLIBC__
#ifndef __GLIBC_PREREQ
#define __GLIBC_PREREQ(maj,min) 0
#endif

#if defined __GLIBC__ && !__GLIBC_PREREQ(2,36)
DIR *opendir (const char *__name) __nonnull ((1));
#else
DIR *__opendir (const char *__name) __nonnull ((1))
     __attribute_malloc__ __attr_dealloc (closedir, 1);
#endif
weak_alias(opendir, __opendir);
#endif
#endif
