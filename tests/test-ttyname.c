/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	int fd = -1;
	char *s;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	fd = strtoul(argv[1], NULL, 0);

	s = ttyname(fd);
	if (!s) {
		perror("ttyname");
		return EXIT_FAILURE;
	}

	printf("%s\n", s);

	return EXIT_SUCCESS;
}
