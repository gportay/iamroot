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

int umount2(const char *target, int flags)
{
	(void)target;
	(void)flags;

	__verbose_func("%s(target: '%s', ...)\n", __func__, target);

	return 0;
}
