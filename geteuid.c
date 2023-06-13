/*
 * Copyright 2020-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

uid_t geteuid(void)
{
	const int errno_save = errno;
	unsigned long ul;
	uid_t ret;

	errno = 0;
	ul = strtoul(__getenv("EUID") ?: "0", NULL, 0);
	if (errno)
		ul = 0;

	/* Not forwarding function */
	ret = __set_errno(errno_save, ul);

	__debug("%s() -> %i\n", __func__, ret);

	return ret;
}
