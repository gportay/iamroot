/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <dirent.h>

int main(int argc, char * const argv[])
{
	struct dirent **namelist;
	int n;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	n = scandir(argv[1], &namelist, NULL, alphasort);
	if (n == -1) {
		perror("scandir");
		return EXIT_FAILURE;
	}

	while (n--)
		free(namelist[n]);
	free(namelist);

	return EXIT_SUCCESS;
}
