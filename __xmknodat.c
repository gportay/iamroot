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

#include "fpath_resolutionat.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int next___xmknodat(int ver, int fd, const char *path, mode_t mode, dev_t dev)
{
	int (*sym)(int, int, const char *, mode_t, dev_t);

	sym = dlsym(RTLD_NEXT, "__xmknodat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(ver, fd, path, mode, dev);
}

int __xmknodat(int ver, int fd, const char *path, mode_t mode, dev_t dev)
{
	const char *real_path;
	char buf[PATH_MAX];
	(void)ver;
	(void)dev;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__fprintf(stderr, "%s(fd %i, path: '%s' -> '%s')\n", __func__, fd, path,
			real_path);

	fd = creat(real_path, mode);
	if (fd == -1)
		return -1;

	if (close(fd))
		perror("close");

	errno = 0;
	return 0;
}
