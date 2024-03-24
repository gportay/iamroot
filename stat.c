/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __NetBSD__
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

int stat(const char *path, struct stat *statbuf)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	/* Forward to another function */
	return fstatat(AT_FDCWD, path, statbuf, 0);
}
#endif
