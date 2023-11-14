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

int __path_setenv(const char *root, const char *name, const char *value,
		 int overwrite)
{
	(void)root;
	(void)name;
	(void)value;
	(void)overwrite;

	return __set_errno(ENOSYS, -1);
}

ssize_t path_resolution(int dfd, const char *path, char *buf, size_t bufsiz,
			int atflags)
{
	(void)dfd;
	(void)path;
	(void)buf;
	(void)bufsiz;
	(void)atflags;

	return __set_errno(ENOSYS, -1);
}
