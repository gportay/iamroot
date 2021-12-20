/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <spawn.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_posix_spawn(pid_t *pid, const char *path,
		     const posix_spawn_file_actions_t *file_actions,
		     const posix_spawnattr_t *attrp,
		     char * const argv[], char * const envp[])
{
	int (*sym)(pid_t *, const char *, const posix_spawn_file_actions_t *,
		   const posix_spawnattr_t *, char * const [], char * const []);
	int ret;

	sym = dlsym(RTLD_NEXT, "posix_spawn");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(pid, path, file_actions, attrp, argv, envp);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int posix_spawn(pid_t *pid, const char *path,
		const posix_spawn_file_actions_t *file_actions,
		const posix_spawnattr_t *attrp,
		char * const argv[], char * const envp[])
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_posix_spawn(pid, real_path, file_actions, attrp, argv,
				envp);
}
