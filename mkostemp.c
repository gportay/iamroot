/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __FreeBSD__
#include <stdio.h>
#include <errno.h>

#include <stdlib.h>

#include "iamroot.h"

int mkostemp(char *path, int oflags)
{
	__debug("%s(path: '%s', oflags: 0%o)\n", __func__, path, oflags);

	/* Forward to another function */
	return mkostemps(path, 0, oflags);
}

#ifdef __GLIBC__
weak_alias(mkostemp, mkostemp64);
#endif
#endif
