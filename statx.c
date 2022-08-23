/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "iamroot.h"

#ifdef __GLIBC__
#if __GLIBC_PREREQ(2,28)
extern uid_t next_getegid();
extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next_statx(int dfd, const char *path, int atflags, unsigned int mask,
	       struct statx *statxbuf)
{
	int (*sym)(int, const char *, int, unsigned int, struct statx *);
	int ret;

	sym = dlsym(RTLD_NEXT, "statx");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(dfd, path, atflags, mask, statxbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int statx(int dfd, const char *path, int atflags, unsigned int mask,
	  struct statx *statxbuf)
{
	char buf[PATH_MAX];
	ssize_t siz;
	uid_t uid;
	gid_t gid;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(dfd: %i, path: '%s' -> '%s', atflags: 0x%x...)\n",
		__func__, dfd, path, buf, atflags);

	__remove_at_empty_path_if_needed(buf, atflags);
	ret = next_statx(dfd, buf, atflags, mask, statxbuf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (statxbuf->stx_uid == uid)
		statxbuf->stx_uid = geteuid();

	gid = next_getegid();
	if (statxbuf->stx_gid == gid)
		statxbuf->stx_gid = 0;

exit:
	return ret;
}
#endif
#endif
