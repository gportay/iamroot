/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <unistd.h>

#include "path_resolution.h"

int chroot(const char *path)
{
	const char *real_path;
	char buf[PATH_MAX];

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s') IAMROOT_ROOT='%s'\n", __func__,
				path, getenv("IAMROOT_ROOT") ?: "");

	real_path = path_resolution(path, buf, sizeof(buf));
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	/* prepend the current working directory for relative paths */
	if (real_path == path) {
		size_t len;
		char *cwd;

		cwd = getcwd(buf, sizeof(buf));
		len = strlen(cwd);
		if (len + 1 + strlen(path) + 1 > sizeof(buf)) {
			errno = ENAMETOOLONG;
			return -1;
		}

		cwd[len++] = '/';
		cwd[len] = 0;
		real_path = strncat(cwd, path, sizeof(buf) - len);
	}

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s') IAMROOT_ROOT='%s'\n", __func__,
				real_path, getenv("IAMROOT_ROOT") ?: "");

	return setenv("IAMROOT_ROOT", real_path, 1);
}
