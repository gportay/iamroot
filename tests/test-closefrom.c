/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	int fd;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	fd = strtoul(argv[1], NULL, 0);

	closefrom(fd);

	fprintf(stderr, "%s\n", "stderr");
	fprintf(stdout, "%s\n", "stdout");

	return EXIT_SUCCESS;
}
