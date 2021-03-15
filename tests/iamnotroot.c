/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stddef.h>
#include <errno.h>
#include <fcntl.h>

char *fpath_resolutionat(int fd, const char *path, char *buf, size_t bufsize,
			 int flags)
{
	(void)fd;
	(void)path;
	(void)buf;
	(void)bufsize;
	(void)flags;

	errno = ENOSYS;
	return NULL;
}

char *path_resolution(const char *path, char *buf, size_t bufsize, int flags)
{
	return fpath_resolutionat(AT_FDCWD, path, buf, bufsize, flags);
}
