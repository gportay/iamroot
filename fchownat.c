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
int next_fchownat(int dfd, const char *path, uid_t owner, gid_t group,
		  int atflags)
{
	int (*sym)(int, const char *, uid_t, gid_t, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchownat");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(dfd, path, owner, group, atflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int fchownat(int dfd, const char *path, uid_t owner, gid_t group, int atflags)
{
	const uid_t oldowner = owner;
	const uid_t oldgroup = group;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	owner = __get_uid(buf);
	group = __get_gid(buf);

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', owner: %i, group: %i, atflags: 0x%x)\n",
		__func__, dfd, __fpath(dfd), path, buf, owner, group, atflags);

	__remove_at_empty_path_if_needed(buf, atflags);
	ret = next_fchownat(dfd, buf, owner, group, atflags);
	__ignore_error_and_warn(ret, dfd, path, atflags);
	/* Force ignoring EPERM error if not chroot'ed */
	if ((ret == -1) && (errno == EPERM))
		ret = __set_errno(0, 0);
	__set_uid(buf, oldowner, owner);
	__set_gid(buf, oldgroup, group);

	return ret;
}
