/*
 * Copyright 2020 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <unistd.h>

static const char *path_resolution(const char *path, char *buf, size_t bufsize)
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

		return buf;
	}

	return path;
}

int chroot(const char *path)
{
	const char *real_path;
	char buf[PATH_MAX];

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s')\n", __func__, path);

	real_path = path_resolution(path, buf, sizeof(buf));
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	return setenv("IAMROOT_ROOT", real_path, 1);
}
