/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>

#include "iamroot.h"

int umount(const char *target)
{
	(void)target;

	__debug("%s(target: '%s', ...)\n", __func__, target);

	return 0;
}
