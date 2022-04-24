/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next_lchown(const char *path, uid_t owner, gid_t group)
{
	int (*sym)(const char *, uid_t, gid_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "lchown");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, owner, group);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int lchown(const char *path, uid_t owner, gid_t group)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	owner = next_geteuid();
	group = getegid();

	__debug("%s(path: '%s' -> '%s', owner: %i, group: %i)\n", __func__,
		path, buf, owner, group);

	ret = next_lchown(buf, owner, group);
	__ignore_error_and_warn(ret, AT_FDCWD, path, 0);

	return ret;
}
