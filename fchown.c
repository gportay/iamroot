/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
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
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, owner, group);
	if (ret == -1)
		__fperror(fd, __func__);

	return ret;
}

int fchown(int fd, uid_t owner, gid_t group)
{
	owner = next_geteuid();
	group = getegid();

	__verbose("%s(fd: %i, owner: %i, group: %i)\n", __func__, fd, owner,
		  group);

	return next_fchown(fd, owner, group);
}
