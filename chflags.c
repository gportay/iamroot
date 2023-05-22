/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined __FreeBSD__ || defined __OpenBSD__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/stat.h>
#include <unistd.h>

#include "iamroot.h"

#ifdef __OpenBSD__
static int (*sym)(const char *, int long);
#else
static int (*sym)(const char *, unsigned long);
#endif

__attribute__((visibility("hidden")))
#ifdef __OpenBSD__
int next_chflags(const char *path, unsigned int flags)
#else
int next_chflags(const char *path, unsigned long flags)
#endif
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "chflags");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, flags);
}

#ifdef __OpenBSD__
int chflags(const char *path, unsigned int flags)
#else
int chflags(const char *path, unsigned long flags)
#endif
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_chflags(path, flags);

	__debug("%s(path: '%s', ...) -> %i\n", __func__, path, ret);

	return ret;
}
#endif
