/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "iamroot.h"

int __debug_fd()
{
	return STDERR_FILENO;
}

int pathsetenv(const char *root, const char *name, const char *value,
	       int overwrite)
{
	(void)root;
	(void)name;
	(void)value;
	(void)overwrite;

	errno = ENOSYS;
	return -1;
}

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
