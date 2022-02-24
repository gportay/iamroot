/*
 * Copyright 2020-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>

#include <unistd.h>

#include "iamroot.h"

extern char **__environ;

int execv(const char *path, char * const argv[])
{
	__debug("%s(path: '%s', argv: { '%s', '%s', ... })\n", __func__, path,
		argv[0], argv[1]);

	return execvpe(path, argv, __environ);
}
