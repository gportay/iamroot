/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	uid_t euid;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	euid = strtoul(argv[1], NULL, 0);

	if (seteuid(euid) == -1) {
		perror("seteuid");
		return EXIT_FAILURE;
	}

	printf("%s\n", getenv("IAMROOT_EUID") ?: "");

	return EXIT_SUCCESS;
}
