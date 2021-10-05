/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>

#include "iamroot.h"

void __perror2(const char *oldpath, const char *newpath, const char *s)
{
	if (errno != EPERM)
		return;

	__verbose("Warning: %s: %s: %s: %m\n", oldpath, newpath, s);
}
