/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <errno.h>

#include <stdio.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	FILE *f;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	f = fopen(argv[1], argv[2]);
	if (!f) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
