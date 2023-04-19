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
	uid_t ruid, euid, suid;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	ruid = strtoul(argv[1], NULL, 0);
	euid = strtoul(argv[2], NULL, 0);
	suid = strtoul(argv[3], NULL, 0);

	if (setresuid(ruid, euid, suid) == -1) {
		perror("setresuid");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
