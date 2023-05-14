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

int swapoff(const char *path)
{
	int ret;
	(void)path;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(path: '%s') -> %ii\n", __func__, path, ret);

	return ret;
}
#endif
