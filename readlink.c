/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

ssize_t readlink(const char *path, char *buf, size_t bufsize)
{
	__debug("%s(path: '%s'...)\n", __func__, path);

	return readlinkat(AT_FDCWD, path, buf, bufsize);
}
