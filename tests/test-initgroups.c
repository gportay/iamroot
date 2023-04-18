/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <grp.h>

int main(int argc, char * const argv[])
{
	const char *user;
	gid_t gid;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	user = argv[1];
	gid = strtoul(argv[2], NULL, 0);

	if (initgroups(user, gid) == -1) {
		perror("initgroups");
		return EXIT_FAILURE;
	}

	printf("%s\n", getenv("IAMROOT_GROUPS"));

	return EXIT_SUCCESS;
}
