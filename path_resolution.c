/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "path_resolution.h"

const char *path_resolution(const char *path, char *buf, size_t bufsize)
{
	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s')\n", __func__, path);

	if (!path || !*path) {
		errno = EINVAL;
		return NULL;
	}
	
	if (*path == '/') {
		char *root = getenv("IAMROOT_ROOT") ?: "";
		int size = snprintf(buf, bufsize, "%s%s", root, path);
		if (size < 0) {
			errno = EINVAL;
			return NULL;
		}

		if ((size_t) size >= bufsize) {
			errno = ENAMETOOLONG;
			return NULL;
		}

		if (getenv("IAMROOT_DEBUG"))
			fprintf(stderr, "%s(buf: '%s')\n", __func__, buf);

		return buf;
	}

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s')\n", __func__, path);

	return path;
}
