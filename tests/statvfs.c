/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/statvfs.h>

int main(int argc, char * const argv[])
{
	struct statvfs buf;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (statvfs(argv[1], &buf)) {
		perror("statvfs");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
