/*
 * Copyright 2021-2023 Gaël PORTAY
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
#ifdef __FreeBSD__
#include <sys/extattr.h>
#endif

#include <unistd.h>

#include "iamroot.h"

static int (*sym)(const char *, uid_t, gid_t);

__attribute__((visibility("hidden")))
int next_chown(const char *path, uid_t owner, gid_t group)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "chown");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, owner, group);
}

int chown(const char *path, uid_t owner, gid_t group)
{
	const uid_t oldowner = owner;
	const uid_t oldgroup = group;
	char buf[PATH_MAX];
	int ret = 1;
	ssize_t siz;
	(void)oldowner;
	(void)oldgroup;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	owner = __get_uid(buf);
	group = __get_gid(buf);

	ret = next_chown(buf, owner, group);
	__ignore_error_and_warn(ret, AT_FDCWD, path, 0);
	/* Force ignoring EPERM error if not chroot'ed */
	if ((ret == -1) && (errno == EPERM))
		ret = __set_errno(0, 0);
	__set_uid(buf, oldowner, owner);
	__set_gid(buf, oldgroup, group);

exit:
	__debug("%s(path: '%s' -> '%s', owner: %i -> %i, group: %i -> %i) -> %i\n",
		__func__, path, buf, oldowner, owner, oldgroup, group, ret);

	return ret;
}
