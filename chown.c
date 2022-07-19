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

int chown(const char *path, uid_t owner, gid_t group)
{
	__debug("%s(path: '%s', owner: %i, group: %i)\n", __func__, path,
		owner, group);

	return fchownat(AT_FDCWD, path, owner, group, 0);
}
