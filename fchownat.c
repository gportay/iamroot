/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>

#include "fpath_resolutionat.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern uid_t next_geteuid();

int next_fchownat(int fd, const char *path, uid_t owner, gid_t group, int flags)
{
	int (*sym)(int, const char *, uid_t, gid_t, int);

	sym = dlsym(RTLD_NEXT, "fchownat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, path, owner, group, flags);
}

int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flags)
{
	const char *real_path;
	char buf[PATH_MAX];

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	owner = next_geteuid();
	group = getegid();

	__fprintf(stderr, "%s(fd: %i, path: '%s' -> '%s', owner: %i, group: %i)\n",
			  __func__, fd, path, real_path, owner, group);

	return next_fchownat(fd, real_path, owner, group, flags);
}
