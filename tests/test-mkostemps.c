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
	int length;
	int oflags;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	memset(buf, 0, sizeof(buf));
	strncpy(buf, argv[1], sizeof(buf)-1);

	length = strtol(argv[2], NULL, 0);
	oflags = strtol(argv[3], NULL, 0);

	if (mkostemps(buf, length, oflags) == -1) {
		perror("mkostemps");
		return EXIT_FAILURE;
	}

	printf("%s\n", buf);

	return EXIT_SUCCESS;
}
