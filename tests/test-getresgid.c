/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	gid_t rgid, egid, sgid;
	(void)argv;

	if (argc < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 1) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (getresgid(&rgid, &egid, &sgid)) {
		perror("getresgid");
		return EXIT_FAILURE;
	}

	printf("%u:%u:%u\n", rgid, egid, sgid);

	return EXIT_SUCCESS;
}
