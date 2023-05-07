/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include "iamroot.h"

int acct(const char *path)
{
	(void)path;

	__debug("%s(path: '%s')\n", __func__, path);

	return 0;
}
