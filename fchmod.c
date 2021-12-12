/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

int fchmod(int fd, mode_t mode)
{
	__debug("%s(fd: %i, mode: 0%03o)\n", __func__, fd, mode);

	return fchmodat(fd, "", mode, 0);
}
