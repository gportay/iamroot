/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>

#include <unistd.h>

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int execvp(const char *file, char * const argv[])
{
	__fprintf(stderr, "%s(file: '%s', argv: '%s'...)\n", __func__, file,
			  argv[0]);

	return execvpe(file, argv, environ);
}
