/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <fcntl.h>

#include <stdio.h>

#include "iamroot.h"

int rename(const char *oldpath, const char *newpath)
{
	__debug("%s(oldpath: '%s', newpath: '%s')\n", __func__, oldpath,
		newpath);

	/* Forward to another function */
	return renameat(AT_FDCWD, oldpath, AT_FDCWD, newpath);
}
