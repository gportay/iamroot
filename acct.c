/*
 * Copyright 2022-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int acct(const char *path)
{
	int ret;
	(void)path;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(path: '%s') -> %i\n", __func__, path, ret);

	return ret;
}
