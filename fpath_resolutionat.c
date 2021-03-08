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
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include <fcntl.h>

#include "fpath_resolutionat.h"

#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern ssize_t next_readlink(const char *, char *, size_t);
extern int next_lstat(const char *, struct stat *);
extern void __procfdname(char *, unsigned);
extern const char *path_resolution(const char *path, char *buf, size_t bufsize,
				   int flags);

static inline ssize_t __procfdreadlink(int fd, char *buf, size_t bufsize)
{
	char tmp[sizeof("/proc/self/fd/") + 4];
	__procfdname(tmp, fd);
	return readlink(tmp, buf, bufsize);
}

char *sanitize(char *path, size_t bufsize)
{
	ssize_t len;

	len = strnlen(path, bufsize);
	while ((len > 3) && (__strncmp(path, "./") == 0)) {
		char *s;
		for (s = path; *s; s++)
			*s = *(s+2);
		len -= 2;
	}
	while ((len > 1) && (path[len-1] == '/')) {
		path[len-1] = 0;
		len--;
	}

	return path;
}

const char *fpath_resolutionat(int fd, const char *path, char *buf,
			       size_t bufsize, int flags)
{
	struct stat statbuf;

	if (fd == -1 || !path || !*path) {
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
	} else if (fd != AT_FDCWD) {
		char dir[PATH_MAX];
		ssize_t siz;
		int size;

		siz = __procfdreadlink(fd, dir, sizeof(dir));
		if (siz == -1) {
			perror("__procfdreadlink");
			return NULL;
		}

		dir[siz] = 0;
		size = snprintf(buf, bufsize, "%s/%s", dir, path);
		if (size < 0) {
			errno = EINVAL;
			return NULL;
		}

		if ((size_t)size >= bufsize) {
			errno = ENAMETOOLONG;
			return NULL;
		}

		path = buf;
	} else {
		strncpy(buf, path, bufsize);

		path = buf;
	}

	path = sanitize(buf, bufsize);

	if (flags == AT_SYMLINK_NOFOLLOW)
		goto exit;

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
			return path_resolution(tmp, buf, bufsize, flags);
	}

exit:
	__fprintf(stderr, "%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd,
			  path, buf);

	return path;
}
