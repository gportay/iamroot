/*
 * Copyright 2021 Gaël PORTAY
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
	(void)flags;

	__debug("%s(flags: 0x%x)\n", __func__, flags);

	return 0;
}
