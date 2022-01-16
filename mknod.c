/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

int mknod(const char *path, mode_t mode, dev_t dev)
{
	__debug("%s(path: '%s', mode: 0%03o)\n", __func__, path, mode);

	return mknodat(AT_FDCWD, path, mode, dev);
}
