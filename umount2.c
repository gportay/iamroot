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
	int ret;
	(void)target;
	(void)flags;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(target: '%s', flags: 0x%x) -> %i\n", __func__, target,
		flags, ret);

	return ret;
}
