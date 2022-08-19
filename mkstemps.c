/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include <stdlib.h>

#include "iamroot.h"

int mkstemps(char *path, int suffixlen)
{
	__debug("%s(path: '%s', ...)\n", __func__, path);

	return mkostemps(path, suffixlen, 0);
}

#ifdef __GLIBC__
weak_alias(mkstemps, mkstemps64);
#endif
