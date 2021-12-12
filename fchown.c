/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

int fchown(int fd, uid_t owner, gid_t group)
{
	__debug("%s(fd: %i, owner: %i, group: %i)\n", __func__, fd, owner,
		group);

	return fchownat(fd, "", owner, group, 0);
}
