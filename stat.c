/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

int stat(const char *path, struct stat *statbuf)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	return fstatat(AT_FDCWD, path, statbuf, 0);
}
