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
#include <unistd.h>

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_symlinkat(const char *string, int fd, const char *path)
{
	int (*sym)(const char *, int, const char *);

	sym = dlsym(RTLD_NEXT, "symlinkat");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(string, fd, path);
}

int symlinkat(const char *string, int fd, const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__fprintf(stderr, "%s(string: '%s', fd: %i, path: '%s' -> '%s')\n",
			  __func__, string, fd, path, real_path);

	return next_symlinkat(string, fd, real_path);
}
