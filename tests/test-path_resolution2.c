/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	int prflags, dfd = AT_FDCWD, ret = EXIT_FAILURE;
	char buf[PATH_MAX];

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	prflags = strtol(argv[3], NULL, 0);

	if (!__strneq(argv[1], "-")) {
		dfd = open(".", O_DIRECTORY);
		if (dfd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (path_resolution2(dfd, argv[2], buf, sizeof(buf), 0,
			     prflags) == -1) {
		perror("path_resolution2");
		goto exit;
	}

	printf("%s\n", buf);

	ret = EXIT_SUCCESS;

exit:
	if (dfd != AT_FDCWD)
		if (close(dfd))
			perror("close");

	return ret;
}
