/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/statfs.h>

int main(int argc, char * const argv[])
{
	struct statfs buf;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (statfs(argv[1], &buf)) {
		perror("statfs");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
