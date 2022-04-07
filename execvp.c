/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>

#include <unistd.h>

#include "iamroot.h"

int execvp(const char *file, char * const argv[])
{
	__debug("%s(file: '%s', argv: { '%s', '%s', ... })\n", __func__, file,
		argv[0], argv[1]);

	return execvpe(file, argv, __environ);
}
