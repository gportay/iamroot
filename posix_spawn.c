/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <spawn.h>

#include "iamroot.h"

extern int __posix_spawn(pid_t *, const char *,
			 const posix_spawn_file_actions_t *,
			 const posix_spawnattr_t *, char * const [],
			 char * const []);

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
		__dlperror(__func__);
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
	return __posix_spawn(pid, path, file_actions, attrp, argv, envp);
}
