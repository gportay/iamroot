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
int next_chown(const char *path, uid_t owner, gid_t group)
{
	int (*sym)(const char *, uid_t, gid_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "chown");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, owner, group);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
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

	__verbose_func("%s(path: '%s' -> '%s', owner: %i, group: %i)\n",
		       __func__, path, real_path, owner, group);

	return next_chown(real_path, owner, group);
}
