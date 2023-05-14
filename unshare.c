/*
 * Copyright 2021,2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include <sched.h>

#include "iamroot.h"

int unshare(int flags)
{
	int ret;
	(void)flags;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(flags: 0x%x) -> %i\n", __func__, flags, ret);

	return ret;
}
