/*
 * Copyright 2021-2023 Gaël PORTAY
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

int __getdebug_fd()
{
	return STDERR_FILENO;
}

int __pathsetenv(const char *root, const char *name, const char *value,
		 int overwrite)
{
	(void)root;
	(void)name;
	(void)value;
	(void)overwrite;

	errno = ENOSYS;
	return -1;
}

ssize_t path_resolution(int dfd, const char *path, char *buf, size_t bufsize,
			int atflags)
{
	(void)dfd;
	(void)path;
	(void)buf;
	(void)bufsize;
	(void)atflags;

	errno = ENOSYS;
	return -1;
}
