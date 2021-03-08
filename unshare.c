/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <sched.h>

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int unshare(int flags)
{
	__fprintf(stderr, "%s(flags: 0x%x)\n", __func__, flags);

	return 0;
}
