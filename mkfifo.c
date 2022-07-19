/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "iamroot.h"

int mkfifo(const char *path, mode_t mode)
{
	__debug("%s(path: '%s', mode: 0%03o)\n", __func__, path, mode);

	return mkfifoat(AT_FDCWD, path, mode);
}
