/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/time.h>

#include "iamroot.h"

int utimes(const char *path, const struct timeval times[2])
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	return futimesat(AT_FDCWD, path, times);
}
