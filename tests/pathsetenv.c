/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (pathsetenv(getenv(argc > 3 ? argv[3] : "IAMROOT_ROOT"), argv[1],
		       argv[2], 1) == -1) {
		perror("pathsetenv");
		return EXIT_FAILURE;
	}

	printf("%s\n", getenv(argv[1]));

	return EXIT_SUCCESS;
}
