/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include "path_resolution.h"

static inline int real_lstat(const char *path, struct stat *buf)
{
	int (*realsym)(const char *, struct stat *);

	realsym = dlsym(RTLD_NEXT, "lstat");
	if (!realsym) {
		errno = ENOTSUP;
		return -1;
	}

	return realsym(path, buf);
}

static inline ssize_t real_readlink(const char *path, char *buf,
				    size_t bufsize)
{
	ssize_t (*realsym)(const char *, char *, size_t);

	realsym = dlsym(RTLD_NEXT, "readlink");
	if (!realsym) {
		errno = ENOTSUP;
		return -1;
	}

	return realsym(path, buf, bufsize);
}

const char *path_resolution(const char *path, char *buf, size_t bufsize)
{
	struct stat statbuf;

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

		path = buf;
	}

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s')\n", __func__, path);

	if (real_lstat(path, &statbuf) != 0)
		return path;

	if (S_ISLNK(statbuf.st_mode)) {
		char tmp[NAME_MAX];
		ssize_t s;

		s = real_readlink(path, tmp, sizeof(tmp) - 1);
		if (s == -1) {
			perror("readlink");
			return NULL;
		}

		tmp[s] = 0;
		if (*tmp == '/')
			return path_resolution(tmp, buf, bufsize);
	}

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s')\n", __func__, path);

	return path;
}
