/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

gid_t getgid()
{
	const int errno_save = errno;
	unsigned long ul;
	uid_t ret;

	errno = 0;
	ul = strtoul(_getenv("IAMROOT_GID") ?: "0", NULL, 0);
	if (errno)
		ul = 0;

	/* Not forwarding function */
	ret = __set_errno(errno_save, ul);

	__debug("%s() -> %i\n", __func__, ret);

	return ret;
}
