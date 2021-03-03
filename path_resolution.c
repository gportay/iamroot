/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include "path_resolution.h"

#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)

extern int __fprintf(FILE *, const char *, ...);
extern ssize_t next_readlink(const char *, char *, size_t);
extern int next_lstat(const char *, struct stat *);

const char *path_resolution(const char *path, char *buf, size_t bufsize)
{
	struct stat statbuf;

	if (!path || !*path) {
		errno = EINVAL;
		return NULL;
	}

	if ((__strncmp(path, "/proc/") == 0) ||
	    (__strncmp(path, "/sys/") == 0) ||
	    (__strncmp(path, "/dev/") == 0))
		return path;

	if (*path == '/') {
		char *root = getenv("IAMROOT_ROOT") ?: "";
		int size = snprintf(buf, bufsize, "%s%s", root, path);
		if (size < 0) {
			errno = EINVAL;
			return NULL;
		}

		if ((size_t)size >= bufsize) {
			errno = ENAMETOOLONG;
			return NULL;
		}

		path = buf;
	}

	if (next_lstat(path, &statbuf) != 0)
		goto exit;

	if (S_ISLNK(statbuf.st_mode)) {
		char tmp[NAME_MAX];
		ssize_t s;

		s = next_readlink(path, tmp, sizeof(tmp) - 1);
		if (s == -1) {
			perror("readlink");
			return NULL;
		}

		tmp[s] = 0;
		if (*tmp == '/')
			return path_resolution(tmp, buf, bufsize);
	}

exit:
	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return path;
}
