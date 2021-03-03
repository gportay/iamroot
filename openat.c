/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...);
extern int next_fchdir(int);
extern int next_open(const char *, int, mode_t);
extern char *next_getcwd(const char *, size_t);

static inline char *getdir(int fd, char *buf, size_t bufsize)
{
	char *ret = NULL;
	int cwdfd = -1;

	if (fd == AT_FDCWD)
		goto getcwd;

	cwdfd = next_open(".", O_RDONLY|O_DIRECTORY, 0);
	if (cwdfd == -1)
		return NULL;

	if (fchdir(fd))
		goto close;

getcwd:
	ret = next_getcwd(buf, bufsize);

close:
	if (cwdfd != -1)
		if (fchdir(cwdfd))
			perror("fchdir");

	return ret;
}

int next_openat(int fd, const char *path, int flags, mode_t mode)
{
	int (*sym)(int, const char *, int, ...);

	sym = dlsym(RTLD_NEXT, "openat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, path, flags, mode);
}

int openat(int fd, const char *path, int flags, ...)
{
	const char *real_path = path;
	char buf[PATH_MAX];
	mode_t mode = 0;

	if (*path == '/') {
		real_path = path_resolution(path, buf, sizeof(buf));
		if (!real_path) {
			perror("path_resolution");
			return -1;
		}
	} else {
		char tmp[PATH_MAX];
		char *dir;
		int size;

		dir = getdir(fd, tmp, sizeof(tmp));
		if (dir == NULL) {
			perror("getcwd");
			return -1;
		}

		size = snprintf(buf, sizeof(buf), "%s/%s", dir, path);
		if (size < 0) {
			errno = EINVAL;
			return -1;
		}

		if ((size_t)size >= sizeof(buf)) {
			errno = ENAMETOOLONG;
			return -1;
		}
	}

	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	__fprintf(stderr, "%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd,
			  path, real_path);

	return next_openat(fd, real_path, flags, mode);
}
