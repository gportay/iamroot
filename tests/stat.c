/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>

int main(int argc, char * const argv[])
{
	struct stat statbuf;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (stat(argv[1], &statbuf)) {
		perror("stat");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
