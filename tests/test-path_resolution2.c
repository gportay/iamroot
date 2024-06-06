/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	char buf[PATH_MAX];
	int prflags;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	prflags = strtol(argv[2], NULL, 0);

	if (path_resolution2(AT_FDCWD, argv[1], buf, sizeof(buf), 0,
			     prflags) == -1) {
		perror("path_resolution2");
		return EXIT_FAILURE;
	}

	printf("%s\n", buf);

	return EXIT_SUCCESS;
}
