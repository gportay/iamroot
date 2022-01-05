/*
 * Copyright 2021-2022 Gaël PORTAY
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
int next_mknod(const char *path, mode_t mode, dev_t dev)
{
	int (*sym)(const char *, mode_t, dev_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "mknod");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, mode, dev);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int mknod(const char *path, mode_t mode, dev_t dev)
{
	char buf[PATH_MAX];
	char *real_path;
	int fd;
	(void)dev;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', mode: 0%03o)\n", __func__, path,
		real_path, mode);
	__warn_if_insuffisant_user_mode(real_path, mode);

	fd = next_creat(path, mode);
	if (fd == -1)
		return -1;

	if (close(fd))
		__fpathperror(fd, "close");

	errno = 0;
	return 0;
}
