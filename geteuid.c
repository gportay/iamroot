/*
 * Copyright 2020 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

uid_t geteuid(void)
{
	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s()\n", __func__);

	return 0;
}
