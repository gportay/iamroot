/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>

#include "iamroot.h"

int umount2(const char *target, int flags)
{
	(void)flags;

	__verbose("%s(target: '%s', ...)\n", __func__, target);

	return 0;
}
