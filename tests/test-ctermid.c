/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include <stdio.h>

int main(int argc, char * const argv[])
{
	char *s;
	(void)argv;

	if (argc < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 1) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	s = ctermid(NULL);
	if (!s) {
		perror("ctermid");
		return EXIT_FAILURE;
	}

	printf("%s\n", s);

	return EXIT_SUCCESS;
}
