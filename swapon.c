/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>

#include <sys/swap.h>

#include "iamroot.h"

int swapon(const char *path, int swapflags)
{
	int ret;
	(void)path;
	(void)swapflags;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(path: '%s', ...) -> %i\n", __func__, path, ret);

	return ret;
}
#endif
