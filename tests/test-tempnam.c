/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

int main(int argc, char * const argv[])
{
	const char *dir = NULL, *pfx = NULL;
	char *s;

	if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc >= 1)
		dir = argv[1];

	if (argc >= 2)
		pfx = argv[2];

	s = tempnam(dir, pfx);
	if (!s) {
		perror("tempnam");
		return EXIT_FAILURE;
	}

	printf("%s\n", s);
	free(s);

	return EXIT_SUCCESS;
}
