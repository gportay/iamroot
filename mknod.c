/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int next_mknod(const char *path, mode_t mode, dev_t dev)
{
	int (*sym)(const char *, mode_t, dev_t);

	sym = dlsym(RTLD_NEXT, "mknod");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, mode, dev);
}

int mknod(const char *path, mode_t mode, dev_t dev)
{
	const char *real_path;
	char buf[PATH_MAX];
	int fd;
	(void)dev;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			real_path);

	fd = creat(path, mode);
	if (fd == -1)
		return -1;

	if (close(fd))
		perror("close");

	errno = 0;
	return 0;
}
