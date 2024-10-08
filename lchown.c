/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif
#if defined __FreeBSD__ || defined __NetBSD__
#include <sys/extattr.h>
#endif

#include <unistd.h>

#include "iamroot.h"

static int (*sym)(const char *, uid_t, gid_t);

hidden int next_lchown(const char *path, uid_t owner, gid_t group)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "lchown");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, owner, group);
}

int lchown(const char *path, uid_t owner, gid_t group)
{
	const int errno_save = errno;
	const uid_t oldowner = owner;
	const uid_t oldgroup = group;
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	(void)oldowner;
	(void)oldgroup;

	/*
	 * According to symlink(7):
	 *
	 * Various system calls do not follow links in the basename component
	 * of a pathname, and operate on the symbolic link itself. They are:
	 * lchown(2), lgetxattr(2), llistxattr(2), lremovexattr(2),
	 * lsetxattr(2), lstat(2), readlink(2), rename(2), rmdir(2), and
	 * unlink(2).
	 */
	siz = path_resolution2(AT_FDCWD, path, buf, sizeof(buf),
			       AT_SYMLINK_NOFOLLOW,
			       PATH_RESOLUTION_NOWALKALONG);
	if (siz == -1)
		goto exit;

	owner = __get_uid(buf);
	if (owner == (uid_t)-1)
		owner = oldowner;

	group = __get_gid(buf);
	if (group == (gid_t)-1)
		group = oldgroup;

	ret = next_lchown(buf, owner, group);
	/* Force ignoring EPERM error if not chroot'ed */
	if ((ret == -1) && (errno == EPERM))
		ret = __set_errno(errno_save, 0);
	__set_uid(buf, oldowner, owner);
	__set_gid(buf, oldgroup, group);

exit:
	__debug("%s(path: '%s' -> '%s', owner: %i -> %i, group: %i -> %i) -> %i\n",
		__func__, path, buf, oldowner, owner, oldgroup, group, ret);

	return ret;
}
