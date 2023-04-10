/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/stat.h>
#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_chflagsat(int dfd, const char *path, unsigned long flags, int atflag)
{
	int (*sym)(int, const char *, unsigned long, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "chflagsat");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(dfd, path, flags, atflag);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int chflagsat(int dfd, const char *path, unsigned long flags, int atflag)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflag);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(dfd: %i <-> '%s', path: '%s', ..., flags: 0x%x)\n", __func__,
		dfd, __fpath(dfd), path, atflag);

	return next_chflagsat(dfd, path, flags, atflag);
}
#endif
