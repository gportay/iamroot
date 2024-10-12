/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

int __xmknodat(int ver, int dfd, const char *path, mode_t mode, dev_t *dev)
{
	int fd, ret = -1;
	(void)ver;
	(void)dev;

	/* Forward to another function */
	fd = openat(dfd, path, O_CREAT|O_WRONLY|O_TRUNC, mode);
	if (fd == -1)
		goto exit;

	__close(fd);

	ret = 0;

exit:
	__debug("%s(dfd: %i, path: '%s', mode: 0%03o) -> %i\n", __func__, dfd,
		path, mode, ret);

	return ret;
}
#endif
