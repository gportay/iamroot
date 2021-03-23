/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <spawn.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_posix_spawnp(pid_t *pid, const char *path,
		      const posix_spawn_file_actions_t *file_actions,
		      const posix_spawnattr_t *attrp,
		      char * const argv[], char * const envp[])
{
	int (*sym)(pid_t *, const char *, const posix_spawn_file_actions_t *,
		   const posix_spawnattr_t *, char * const [], char * const []);

	sym = dlsym(RTLD_NEXT, "posix_spawnp");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(pid, path, file_actions, attrp, argv, envp);
}

int posix_spawnp(pid_t *pid, const char *path,
		 const posix_spawn_file_actions_t *file_actions,
		 const posix_spawnattr_t *attrp,
		 char * const argv[], char * const envp[])
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_posix_spawnp(pid, real_path, file_actions, attrp, argv,
				 envp);
}
