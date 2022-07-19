/*
 * Copyright 2022 Gaël PORTAY
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
	(void)path;

	__debug("%s(path: '%s')\n", __func__, path);

	return 0;
}
#endif
