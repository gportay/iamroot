/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	char buf[PATH_MAX];

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	memset(buf, 0, sizeof(buf));
	strncpy(buf, argv[1], sizeof(buf)-1);

	if (mkstemp(buf) == -1) {
		perror("mkstemp");
		return EXIT_FAILURE;
	}

	printf("%s\n", buf);

	return EXIT_SUCCESS;
}
