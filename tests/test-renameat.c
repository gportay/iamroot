/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <stdio.h>
#include <fcntl.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	int olddfd = AT_FDCWD, newdfd = AT_FDCWD, ret = EXIT_FAILURE;

	if (argc < 5) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 5) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (!__strneq(argv[1], "-")) {
		olddfd = open(".", O_DIRECTORY);
		if (olddfd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (!__strneq(argv[3], "-")) {
		newdfd = open(".", O_DIRECTORY);
		if (newdfd == -1) {
			perror("open");
			goto exit;
		}
	}

	if (renameat(olddfd, argv[2], newdfd, argv[4])) {
		perror("renameat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (olddfd != AT_FDCWD)
		if (close(olddfd))
			perror("close");

	if (newdfd != AT_FDCWD)
		if (close(newdfd))
			perror("close");

	return ret;
}
