/*
 * Copyright 2020-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>

#include "iamroot.h"

int umount(const char *target)
{
	int ret;
	(void)target;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(target: '%s') -> %i\n", __func__, target, ret);

	return ret;
}
#endif
