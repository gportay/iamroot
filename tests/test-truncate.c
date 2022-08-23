/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	off_t length;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	length = strtol(argv[2], NULL, 0);

	if (truncate(argv[1], length) == -1) {
		perror("truncate");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
