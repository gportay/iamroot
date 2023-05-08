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

__attribute__((visibility("hidden")))
#ifdef __OpenBSD__
int next_chflagsat(int dfd, const char *path, unsigned int flags, int atflag)
#else
int next_chflagsat(int dfd, const char *path, unsigned long flags, int atflag)
#endif
{
#ifdef __OpenBSD__
	int (*sym)(int, const char *, unsigned int, int);
#else
	int (*sym)(int, const char *, unsigned long, int);
#endif
	int ret;

	sym = dlsym(RTLD_NEXT, "chflagsat");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(dfd, path, flags, atflag);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

#ifdef __OpenBSD__
int chflagsat(int dfd, const char *path, unsigned int flags, int atflag)
#else
int chflagsat(int dfd, const char *path, unsigned long flags, int atflag)
#endif
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflag);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	__debug("%s(dfd: %i <-> '%s', path: '%s', ..., flags: 0x%x)\n", __func__,
		dfd, __fpath(dfd), path, atflag);

	return next_chflagsat(dfd, path, flags, atflag);
}
#endif
