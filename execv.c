/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

int execv(const char *path, char * const argv[])
{
	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s', argv: '%s'...)\n",
				__func__, path, argv[0]);

	return execvpe(path, argv, environ);
}
