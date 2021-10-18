/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>

#include <unistd.h>

#include "iamroot.h"

int execv(const char *path, char * const argv[])
{
	__verbose("%s(path: '%s', argv: '%s'...)\n", __func__, path, argv[0]);

	return execvpe(path, argv, environ);
}
