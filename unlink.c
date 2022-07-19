/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

int unlink(const char *path)
{
	__debug("%s(path: '%s')\n", __func__, path);

	return unlinkat(AT_FDCWD, path, 0);
}
