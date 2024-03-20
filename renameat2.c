/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <stdio.h>

#include "iamroot.h"

static int (*sym)(int, const char *, int, const char *, unsigned int);

hidden int next_renameat2(int olddfd, const char *oldpath, int newdfd,
			  const char *newpath, unsigned int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "renameat2");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(olddfd, oldpath, newdfd, newpath, flags);
}

int renameat2(int olddfd, const char *oldpath, int newdfd, const char *newpath,
	      unsigned int flags)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(olddfd, oldpath, oldbuf, sizeof(oldbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	siz = path_resolution(newdfd, newpath, newbuf, sizeof(newbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	ret = next_renameat2(olddfd, oldbuf, newdfd, newbuf, flags);

exit:
	__debug("%s(olddfd: %i <-> '%s', oldpath: '%s' -> '%s', newdfd: %i <-> '%s', newpath: '%s' -> '%s', flags: 0x%x) -> %i\n",
		__func__, olddfd, __fpath(olddfd), oldpath, oldbuf, newdfd,
		__fpath2(newdfd), newpath, newbuf, flags, ret);

	return ret;
}
