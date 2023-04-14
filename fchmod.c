/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

int fchmod(int fd, mode_t mode)
{
	int atflags = AT_EMPTY_PATH;

	__debug("%s(fd: %i <-> '%s', mode: 0%03o)\n", __func__, fd,
		__fpath(fd), mode);

#ifdef __linux__
	atflags = 0;
#endif
	return fchmodat(fd, "", mode, atflags);
}
