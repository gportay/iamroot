/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <sys/xattr.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

extern int next_creat(const char *, mode_t);

int __xmknodat(int ver, int dfd, const char *path, mode_t mode, dev_t *dev)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	ssize_t siz;
	int fd;
	(void)ver;
	(void)dev;

	siz = path_resolution(dfd, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__warn_if_insuffisant_user_mode(buf, mode);
	__debug("%s(dfd %i, path: '%s' -> '%s', mode: 0%03o -> 0%03o)\n",
		__func__, dfd, path, buf, oldmode, mode);

	fd = next_creat(buf, mode);
	if (fd == -1)
		return -1;
	__close(fd);
	__set_mode(buf, oldmode, mode);

	return 0;
}
#endif
