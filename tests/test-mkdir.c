/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char * const argv[])
{
	mode_t mode;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	mode = strtoul(argv[2], NULL, 0);

	if (mkdir(argv[1], mode)) {
		perror("mkdir");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
