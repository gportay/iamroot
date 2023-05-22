/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#ifdef __linux__
#include <sys/xattr.h>
#endif
#ifdef __FreeBSD__
#include <sys/extattr.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

extern int next_creat(const char *, mode_t);

int mknodat(int dfd, const char *path, mode_t mode, dev_t dev)
{
	const mode_t oldmode = mode;
	char buf[PATH_MAX];
	int fd, ret = -1;
	ssize_t siz;
	(void)oldmode;
	(void)dev;

	siz = path_resolution(dfd, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	__fwarn_if_insuffisant_user_modeat(dfd, buf, mode, 0);

	ret = next_creat(buf, mode);
	if (ret == -1)
		goto exit;

	fd = ret;
	ret = 0;
	__close(fd);
	__set_mode(buf, oldmode, mode);

exit:
	__debug("%s(dfd %i <-> '%s', path: '%s' -> '%s', mode: 0%03o -> 0%03o) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, oldmode, mode, ret);

	return ret;
}
