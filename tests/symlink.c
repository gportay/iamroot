/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (symlink(argv[1], argv[2])) {
		perror("symlink");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
