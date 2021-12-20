/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next_fchown(int fd, uid_t owner, gid_t group)
{
	int (*sym)(int, uid_t, gid_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchown");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, owner, group);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int fchown(int fd, uid_t owner, gid_t group)
{
	char buf[PATH_MAX];
	char *real_path;
	ssize_t siz;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, "__procfdreadlink");
		return -1;
	}
	buf[siz] = 0;
	real_path = buf;

	owner = next_geteuid();
	group = getegid();

	__debug("%s(fd: %i -> '%s', owner: %i, group: %i)\n", __func__, fd,
		real_path, owner, group);

	return next_fchown(fd, owner, group);
}
