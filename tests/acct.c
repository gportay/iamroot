/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (acct(argv[1]) == -1) {
		perror("acct");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
