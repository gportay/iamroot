/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

extern int next_creat(const char *, mode_t);

__attribute__((visibility("hidden")))
int next___xmknodat(int ver, int fd, const char *path, mode_t mode, dev_t *dev)
{
	int (*sym)(int, int, const char *, mode_t, dev_t *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__xmknodat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(ver, fd, path, mode, dev);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int __xmknodat(int ver, int fd, const char *path, mode_t mode, dev_t *dev)
{
	char buf[PATH_MAX];
	char *real_path;
	(void)ver;
	(void)dev;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(fd %i, path: '%s' -> '%s', mode: 0%03o)\n", __func__, fd,
		  path, real_path, mode);
	__warn_if_insuffisant_user_mode(real_path, mode);

	fd = next_creat(real_path, mode);
	if (fd == -1)
		return -1;

	if (close(fd))
		perror("close");

	errno = 0;
	return 0;
}
