/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

int fchown(int fd, uid_t owner, gid_t group)
{
	__debug("%s(fd: %i <-> '%s', owner: %i, group: %i)\n", __func__, fd,
		__fpath(fd), owner, group);

	return fchownat(fd, "", owner, group, AT_EMPTY_PATH);
}
