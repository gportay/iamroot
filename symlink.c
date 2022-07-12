/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

int symlink(const char *string, const char *path)
{
	__debug("%s(string: '%s': path: '%s')\n", __func__, string, path);

	return symlinkat(string, AT_FDCWD, path);
}
