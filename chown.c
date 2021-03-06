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

extern char *path_resolution(const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern uid_t next_geteuid();

__attribute__((visibility("hidden")))
int next_chown(const char *path, uid_t owner, gid_t group)
{
	int (*sym)(const char *, uid_t, gid_t);

	sym = dlsym(RTLD_NEXT, "chown");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, owner, group);
}

int chown(const char *path, uid_t owner, gid_t group)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	owner = next_geteuid();
	group = getegid();

	__fprintf(stderr, "%s(path: '%s' -> '%s', owner: %i, group: %i)\n",
			  __func__, path, real_path, owner, group);

	return next_chown(real_path, owner, group);
}
