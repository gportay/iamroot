/*
 * Copyright 2020 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>

int umount(const char *target)
{
	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(target: '%s', ...)\n", __func__, target);

	return 0;
}
