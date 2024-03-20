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
static int (*sym)(const char *, int long);
#else
static int (*sym)(const char *, unsigned long);
#endif

#ifdef __OpenBSD__
hidden int next_chflags(const char *path, unsigned int flags)
#else
hidden int next_chflags(const char *path, unsigned long flags)
#endif
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "chflags");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, flags);
}

#ifdef __OpenBSD__
int chflags(const char *path, unsigned int flags)
#else
int chflags(const char *path, unsigned long flags)
#endif
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_chflags(path, flags);

exit:
	__debug("%s(path: '%s', ...) -> %i\n", __func__, path, ret);

	return ret;
}
#endif
