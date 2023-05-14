/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined __FreeBSD__ || defined __OpenBSD__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>

#include "iamroot.h"

int unmount(const char *dir, int flags)
{
	int ret;
	(void)dir;
	(void)flags;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(dir: '%s', flags: 0x%x) -> %i\n", __func__, dir, flags,
		ret);

	return ret;
}
#endif
