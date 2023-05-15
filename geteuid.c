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
	const int save_errno = errno;
	unsigned long ul;

	errno = 0;
	ul = strtoul(__getenv("EUID") ?: "0", NULL, 0);
	if (errno)
		ul = 0;

	__debug("%s(): %lu\n", __func__, ul);

	errno = save_errno;
	return ul;
}
