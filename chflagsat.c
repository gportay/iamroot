/*
 * Copyright 2022-2024 Gaël PORTAY
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
static int (*sym)(int, const char *, unsigned int, int);
#else
static int (*sym)(int, const char *, unsigned long, int);
#endif

hidden
#ifdef __OpenBSD__
int next_chflagsat(int dfd, const char *path, unsigned int flags, int atflag)
#else
int next_chflagsat(int dfd, const char *path, unsigned long flags, int atflag)
#endif
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "chflagsat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, flags, atflag);
}

#ifdef __OpenBSD__
int chflagsat(int dfd, const char *path, unsigned int flags, int atflag)
#else
int chflagsat(int dfd, const char *path, unsigned long flags, int atflag)
#endif
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret = -1;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflag);
	if (siz == -1)
		goto exit;

	ret = next_chflagsat(dfd, path, flags, atflag);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s', ..., flags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, atflag, ret);

	return ret;
}
#endif
