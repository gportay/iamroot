/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "iamroot.h"

#ifndef __GLIBC_PREREQ
#define __GLIBC_PREREQ(maj,min) 0
#endif

#if defined __GLIBC__ && __GLIBC_PREREQ(2,28)
static int (*sym)(int, const char *, int, unsigned int, struct statx *);

hidden int next_statx(int dfd, const char *path, int atflags,
		      unsigned int mask, struct statx *statxbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "statx");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, atflags, mask, statxbuf);
}

int statx(int dfd, const char *path, int atflags, unsigned int mask,
	  struct statx *statxbuf)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	uid_t uid;
	gid_t gid;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

	ret = next_statx(dfd, buf, atflags, mask, statxbuf);
	if (ret == -1)
		goto exit;

	uid = __get_uid(buf);
	if (uid == (uid_t)-1)
		statxbuf->stx_uid = 0;

	gid = __get_gid(buf);
	if (gid == (gid_t)-1)
		statxbuf->stx_gid = 0;

	__stx_mode(buf, statxbuf);
	__stx_uid(buf, statxbuf);
	__stx_gid(buf, statxbuf);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', atflags: 0x%x...) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, atflags, ret);

	return ret;
}
#endif
