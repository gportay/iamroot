/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif
#if defined __FreeBSD__ || defined __NetBSD__
#include <sys/extattr.h>
#endif

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

static int (*sym)(int, const char *, uid_t, gid_t, int);

__attribute__((visibility("hidden")))
int next_fchownat(int dfd, const char *path, uid_t owner, gid_t group,
		  int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fchownat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, owner, group, atflags);
}

int fchownat(int dfd, const char *path, uid_t owner, gid_t group, int atflags)
{
	const uid_t oldowner = owner;
	const uid_t oldgroup = group;
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	(void)oldowner;
	(void)oldgroup;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		goto exit;

	owner = __get_uid(buf);
	group = __get_gid(buf);

	ret = next_fchownat(dfd, buf, owner, group, atflags);
	__ignore_error_and_warn(ret, dfd, path, atflags);
	/* Force ignoring EPERM error if not chroot'ed */
	if ((ret == -1) && (errno == EPERM))
		ret = __set_errno(0, 0);
	__set_uid(buf, oldowner, owner);
	__set_gid(buf, oldgroup, group);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', owner: %i -> %i, group: %i -> %i, atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, oldowner, owner,
		oldgroup, group, atflags, ret);

	return ret;
}
