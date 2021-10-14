/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>

#include "iamroot.h"

void __fperror(int fd, const char *s)
{
	if ((errno != EPERM) && (errno != EACCES))
		return;

	__verbose("Warning: %i: %s: %m\n", fd, s);
}
