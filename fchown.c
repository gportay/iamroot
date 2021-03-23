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

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next_fchown(int fd, uid_t owner, gid_t group)
{
	int (*sym)(int, uid_t, gid_t);

	sym = dlsym(RTLD_NEXT, "fchown");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, owner, group);
}

int fchown(int fd, uid_t owner, gid_t group)
{
	owner = next_geteuid();
	group = getegid();

	__fprintf(stderr, "%s(fd: %i, owner: %i, group: %i)\n", __func__, fd,
			  owner, group);

	return next_fchown(fd, owner, group);
}
