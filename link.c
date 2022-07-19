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

int link(const char *oldpath, const char *newpath)
{
	__debug("%s(oldpath: '%s', newpath: '%s')\n", __func__, oldpath,
		newpath);

	return linkat(AT_FDCWD, oldpath, AT_FDCWD, newpath, 0);
}
