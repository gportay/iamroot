/*
 * Copyright 2021,2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern char *path_sanitize(char *path, size_t bufsize) __attribute__((weak));

int main(int argc, char * const argv[])
{
	int i;

	for (i = 1; i < argc; i++)
		printf("%s\n", path_sanitize(argv[i], strlen(argv[i])));

	return EXIT_SUCCESS;
}
