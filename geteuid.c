/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>

extern int __fprintf(FILE *, const char *, ...);

uid_t geteuid(void)
{
	unsigned long ul;

	errno = 0;
	ul = strtoul(getenv("IAMROOT_GETEUID") ?: "0", NULL, 0);
	if (!errno)
		return ul;

	__fprintf(stderr, "%s(): IAMROOT_GETEUID: %lu\n", __func__, ul);

	return 0;
}
