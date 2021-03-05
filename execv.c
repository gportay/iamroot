/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>

#include <unistd.h>

extern int __fprintf(FILE *, const char *, ...);

int execv(const char *path, char * const argv[])
{
	__fprintf(stderr, "%s(path: '%s', argv: '%s'...)\n", __func__,
			  path, argv[0]);

	return execvpe(path, argv, environ);
}
