/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_mkfifoat(int fd, const char *path, mode_t mode)
{
	int (*sym)(int, const char *, mode_t);

	sym = dlsym(RTLD_NEXT, "mkfifoat");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, path, mode);
}

int mkfifoat(int fd, const char *path, mode_t mode)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__fprintf(stderr, "%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd,
			  path, real_path);

	return next_mkfifoat(fd, real_path, mode);
}
