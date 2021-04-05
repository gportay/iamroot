/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>

#include <sched.h>

#include "iamroot.h"

int unshare(int flags)
{
	__verbose("%s(flags: 0x%x)\n", __func__, flags);

	return 0;
}
