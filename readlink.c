/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	/* Forward to another function */
	return readlinkat(AT_FDCWD, path, buf, bufsiz);
}

ssize_t __readlink_chk(const char *path, char *buf, size_t pathlen,
		       size_t bufsiz)
{
	(void)pathlen;

	__debug("%s(path: '%s', buf: %p, pathlen: %zu, bufsiz: %zu)\n",
		__func__, path, buf, pathlen, bufsiz);

	/* Forward to another function */
	return readlinkat(AT_FDCWD, path, buf, bufsiz);
}
