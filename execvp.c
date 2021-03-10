/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

extern int __fprintf(FILE *, const char *, ...);

int execvp(const char *file, char * const argv[])
{
	__fprintf(stderr, "%s(file: '%s', argv: '%s'...)\n", __func__, file,
			  argv[0]);

	return execvpe(file, argv, environ);
}
