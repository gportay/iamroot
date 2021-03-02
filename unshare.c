/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <sched.h>

int unshare(int flags)
{
	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(flags: 0x%x)\n", __func__, flags);


	return 0;
}
