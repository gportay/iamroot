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

	f = freopen(argv[1], argv[2], stdout);
	if (!f) {
		perror("freopen");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
