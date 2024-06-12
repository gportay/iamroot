/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <paths.h>
#include <limits.h>

#include <spawn.h>

#include "iamroot.h"

#define getenv _getenv

/*
 * Stolen and hacked from musl (src/process/execvp.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static int __posix_spawnp(pid_t *pid, const char *file,
			  const posix_spawn_file_actions_t *file_actions,
			  const posix_spawnattr_t *attrp,
			  char * const argv[], char * const envp[])
{
	const char *p, *z, *path = getenv("PATH");
	size_t l, k;
	int seen_eacces = 0;

	errno = ENOENT;
	if (!*file) return -1;

	if (strchr(file, '/'))
		return posix_spawn(pid, file, file_actions, attrp, argv, envp);

	if (!path) path = _PATH_DEFPATH;
	k = strnlen(file, NAME_MAX+1);
	if (k > NAME_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}
	l = strnlen(path, PATH_MAX-1)+1;

	for(p=path; ; p=z) {
		char b[l+k+1];
		int r;
		z = __strchrnul(p, ':');
		if ((size_t)(z-p) >= l) {
			if (!*z++) break;
			continue;
		}
		memcpy(b, p, z-p);
		b[z-p] = '/';
		memcpy(b+(z-p)+(z>p), file, k+1);
		r = posix_spawn(pid, b, file_actions, attrp, argv, envp);
		if (r == 0)
			return 0;
		switch (errno) {
		case EACCES:
			seen_eacces = 1;
		case ENOENT:
		case ENOTDIR:
			break;
		default:
			return -1;
		}
		if (!*z++) break;
	}
	if (seen_eacces) errno = EACCES;
	return -1;
}

#undef getenv

int posix_spawnp(pid_t *pid, const char *file,
		 const posix_spawn_file_actions_t *file_actions,
		 const posix_spawnattr_t *attrp,
		 char * const argv[], char * const envp[])
{
	__debug("%s(file: '%s', ..., argv: { '%s', '%s', ... }, envp: %p)\n",
		__func__, file, argv[0], argv[1], envp);

	__debug_execve(argv, envp);

	/* Forward to local function */
	return __posix_spawnp(pid, file, file_actions, attrp, argv, envp);
}
