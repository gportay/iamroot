/*
 * Copyright 2021-2022 Gaël PORTAY
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

extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next_fchownat(int fd, const char *path, uid_t owner, gid_t group, int flags)
{
	int (*sym)(int, const char *, uid_t, gid_t, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchownat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, owner, group, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flags)
{
	char buf[PATH_MAX];
	int ret;

	if (path_resolution(fd, path, buf, sizeof(buf), flags) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	owner = next_geteuid();
	group = getegid();

	__debug("%s(fd: %i, path: '%s' -> '%s', owner: %i, group: %i, flags: 0x%x)\n",
		__func__, fd, path, buf, owner, group, flags);

	ret = next_fchownat(fd, buf, owner, group, flags);
	__ignore_error_and_warn(ret, fd, path, flags);

	return ret;
}
