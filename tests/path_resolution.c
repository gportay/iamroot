/*
 * Copyright 2021-2022 Gaël PORTAY
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

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (path_resolution(AT_FDCWD, argv[1], buf, sizeof(buf), 0) == NULL) {
		perror("path_resolution");
		return EXIT_FAILURE;
	}

	printf("%s\n", buf);

	return EXIT_SUCCESS;
}
