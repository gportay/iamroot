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

int euidaccess(const char *path, int mode)
{
	__debug("%s(path: '%s', mode: 0%03o)\n", __func__, path, mode);

	/* Forward to another function */
	return faccessat(AT_FDCWD, path, mode, AT_EACCESS);
}

weak_alias(euidaccess, eaccess);
