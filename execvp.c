/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>

#include <unistd.h>

#include "iamroot.h"

int execvp(const char *file, char * const argv[])
{
	__verbose("%s(file: '%s', argv: '%s'...)\n", __func__, file, argv[0]);

	return execvpe(file, argv, environ);
}
