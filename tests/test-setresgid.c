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
	gid_t rgid, egid, sgid;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	rgid = strtoul(argv[1], NULL, 0);
	egid = strtoul(argv[2], NULL, 0);
	sgid = strtoul(argv[3], NULL, 0);

	if (setresgid(rgid, egid, sgid) == -1) {
		perror("setresgid");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
