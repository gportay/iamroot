/*
 * Copyright 2020 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>

uid_t geteuid(void)
{
	unsigned long ul;

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s()\n", __func__);

	errno = 0;
	ul = strtoul(getenv("IAMROOT_GETEUID") ?: "0", NULL, 0);
	if (!errno)
		return ul;

	return 0;
}
