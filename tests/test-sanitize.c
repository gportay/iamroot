/*
 * Copyright 2021,2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern char *__path_sanitize(char *, size_t) __attribute__((weak));

int main(int argc, char * const argv[])
{
	int i;

	for (i = 1; i < argc; i++)
		printf("%s\n", __path_sanitize(argv[i], strlen(argv[i])));

	return EXIT_SUCCESS;
}
