/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

int execvp(const char *file, char *const argv[])
{
	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(file: '%s', argv: '%s'...)\n",
				__func__, file, argv[0]);

	return execvpe(file, argv, environ);
}
