/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int umount(const char *target)
{
	__fprintf(stderr, "%s(target: '%s', ...)\n", __func__, target);

	return 0;
}
