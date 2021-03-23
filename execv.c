/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>

#include <unistd.h>

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int execv(const char *path, char * const argv[])
{
	__fprintf(stderr, "%s(path: '%s', argv: '%s'...)\n", __func__,
			  path, argv[0]);

	return execvpe(path, argv, environ);
}
