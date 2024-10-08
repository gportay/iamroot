/*
 * Copyright 2021-2024 Gaël PORTAY
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

ssize_t path_resolution2(int dfd, const char *path, char *buf, size_t bufsiz,
			 int atflags, int prflags)
{
	(void)dfd;
	(void)path;
	(void)buf;
	(void)bufsiz;
	(void)atflags;
	(void)prflags;

	return __set_errno(ENOSYS, -1);
}

void unused()
{
}
