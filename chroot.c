/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
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

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s') IAMROOT_ROOT='%s'\n", __func__,
				real_path, getenv("IAMROOT_ROOT") ?: "");

	return setenv("IAMROOT_ROOT", real_path, 1);
}
