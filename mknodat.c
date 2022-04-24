/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

extern int next_creat(const char *, mode_t);

int mknodat(int fd, const char *path, mode_t mode, dev_t dev)
{
	char buf[PATH_MAX];
	ssize_t siz;
	(void)dev;

	siz = path_resolution(fd, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd %i, path: '%s' -> '%s', mode: 0%03o)\n", __func__, fd,
		path, buf, mode);
	__fwarn_if_insuffisant_user_modeat(fd, buf, mode, 0);

	fd = next_creat(buf, mode);
	if (fd == -1)
		return -1;

	__close(fd);

	return 0;
}
