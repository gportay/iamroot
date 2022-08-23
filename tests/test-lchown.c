/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	uid_t owner;
	gid_t group;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	owner = strtoul(argv[2], NULL, 0);
	group = strtoul(argv[3], NULL, 0);

	if (lchown(argv[1], owner, group)) {
		perror("lchown");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
