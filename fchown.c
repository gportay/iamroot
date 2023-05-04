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
#ifdef __FreeBSD__
#include <sys/extattr.h>
#endif

#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fchown(int fd, uid_t owner, gid_t group)
{
	int (*sym)(int, uid_t, gid_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchown");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(fd, owner, group);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fchown(int fd, uid_t owner, gid_t group)
{
	const uid_t oldowner = owner;
	const uid_t oldgroup = group;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return __fpath_perror(fd, -1);

	owner = __get_uid(buf);
	group = __get_gid(buf);

	__debug("%s(fd: %i <-> '%s', owner: %i, group: %i)\n", __func__, fd,
		__fpath(fd), owner, group);

	ret = next_fchown(fd, owner, group);
	__ignore_error_and_warn(ret, AT_FDCWD, buf, 0);
	/* Force ignoring EPERM error if not chroot'ed */
	if ((ret == -1) && (errno == EPERM))
		ret = __set_errno(0, 0);
	__set_uid(buf, oldowner, owner);
	__set_gid(buf, oldgroup, group);

	return ret;
}
