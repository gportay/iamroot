/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __NetBSD__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <fcntl.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif
#ifdef __FreeBSD__
#include <sys/extattr.h>
#endif

#include <sys/stat.h>

#include "iamroot.h"

static int (*sym)(const char *, struct stat *);

hidden int next_lstat(const char *path, struct stat *statbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "lstat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, statbuf);
}

int lstat(const char *path, struct stat *statbuf)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	uid_t uid;
	gid_t gid;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	ret = next_lstat(buf, statbuf);
	if (ret == -1)
		goto exit;

	uid = __get_uid(buf);
	if (uid == (uid_t)-1)
		statbuf->st_uid = 0;

	gid = __get_gid(buf);
	if (gid == (gid_t)-1)
		statbuf->st_gid = 0;

	__st_mode(buf, statbuf);
	__st_uid(buf, statbuf);
	__st_gid(buf, statbuf);

exit:
	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf, ret);

	return ret;
}
#endif
