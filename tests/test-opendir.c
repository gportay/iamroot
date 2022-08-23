/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <dirent.h>

int main(int argc, char * const argv[])
{
	DIR *dir;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	dir = opendir(argv[1]);
	if (!dir) {
		perror("opendir");
		return EXIT_FAILURE;
	}

	if (closedir(dir))
		perror("closedir");

	return EXIT_SUCCESS;
}
