/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_linkat(int olddfd, const char *oldpath, int newdfd,
		const char *newpath, int atflags)
{
	int (*sym)(int, const char *, int, const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "linkat");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(olddfd, oldpath, newdfd, newpath, atflags);
	if (ret == -1)
		__pathperror2(oldpath, newpath, __func__);

	return ret;
}

int linkat(int olddfd, const char *oldpath, int newdfd, const char *newpath,
	   int atflags)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(olddfd, oldpath, oldbuf, sizeof(oldbuf), atflags);
	if (siz == -1) {
		__pathperror(oldpath, __func__);
		return -1;
	}

	siz = path_resolution(newdfd, newpath, newbuf, sizeof(newbuf), 0);
	if (siz == -1) {
		__pathperror(newpath, __func__);
		return -1;
	}

	__debug("%s(olddfd: %i <-> '%s', oldpath: '%s' -> '%s', newdfd: %i <-> '%s', newpath: '%s' -> '%s', atflags: 0x%x)\n",
		__func__, olddfd, __fpath(olddfd), oldpath, oldbuf, newdfd,
		__fpath2(newdfd), newpath, newbuf, atflags);

	__remove_at_empty_path_if_needed(oldbuf, atflags);
	__remove_at_empty_path_if_needed(newbuf, atflags);
	return next_linkat(olddfd, oldbuf, newdfd, newbuf, atflags);
}
