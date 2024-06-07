/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

int mknodat(int dfd, const char *path, mode_t mode, dev_t dev)
{
	int fd;
	(void)dev;

	__debug("%s(dfd: %i, path: '%s', mode: 0%03o)\n", __func__, dfd, path,
		mode);

	/* Forward to another function */
	fd = openat(dfd, path, O_CREAT|O_WRONLY|O_TRUNC, mode);
	if (fd == -1)
		return -1;

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull-compare"
#endif
	__fset_path_resolution(fd, path);
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
	__close(fd);

	return 0;
}
