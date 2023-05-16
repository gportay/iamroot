/*
 * Copyright 2020-2021,2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>

#include "iamroot.h"

int umount2(const char *target, int flags)
{
	(void)target;
	(void)flags;

	__debug("%s(target: '%s', flags: 0x%x)\n", __func__, target, flags);

	return 0;
}
