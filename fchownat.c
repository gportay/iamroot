/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);
extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next_fchownat(int fd, const char *path, uid_t owner, gid_t group, int flags)
{
	int (*sym)(int, const char *, uid_t, gid_t, int);

	sym = dlsym(RTLD_NEXT, "fchownat");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, path, owner, group, flags);
}

int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	owner = next_geteuid();
	group = getegid();

	__verbose("%s(fd: %i, path: '%s' -> '%s', owner: %i, group: %i)\n",
		  __func__, fd, path, real_path, owner, group);

	return next_fchownat(fd, real_path, owner, group, flags);
}
