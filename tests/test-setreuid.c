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
	uid_t ruid, euid;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	ruid = strtoul(argv[1], NULL, 0);
	euid = strtoul(argv[2], NULL, 0);

	if (setreuid(ruid, euid) == -1) {
		perror("setreuid");
		return EXIT_FAILURE;
	}

	printf("%s:%s\n", getenv("IAMROOT_UID") ?: "",
	                  getenv("IAMROOT_EUID") ?: "");

	return EXIT_SUCCESS;
}
