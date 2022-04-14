/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>

#include "iamroot.h"

int unmount(const char *dir, int flags)
{
	(void)dir;
	(void)flags;

	__debug("%s(dir: '%s', ...)\n", __func__, dir);

	return 0;
}
#endif
