/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next_lchown(const char *path, uid_t owner, gid_t group)
{
	int (*sym)(const char *, uid_t, gid_t);

	sym = dlsym(RTLD_NEXT, "lchown");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, owner, group);
}

int lchown(const char *path, uid_t owner, gid_t group)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	owner = next_geteuid();
	group = getegid();

	__fprintf(stderr, "%s(path: '%s' -> '%s', owner: %i, group: %i)\n",
			  __func__, path, real_path, owner, group);

	return next_lchown(real_path, owner, group);
}
