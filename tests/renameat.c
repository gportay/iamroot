/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <fcntl.h>

#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)

int main(int argc, char * const argv[])
{
	int oldfd = AT_FDCWD, newfd = AT_FDCWD, ret = EXIT_FAILURE;

	if (argc < 5) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 5) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (__strncmp(argv[1], "-") != 0) {
		oldfd = open(".", O_DIRECTORY);
		if (oldfd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (__strncmp(argv[3], "-") != 0) {
		newfd = open(".", O_DIRECTORY);
		if (newfd == -1) {
			perror("open");
			goto exit;
		}
	}

	if (renameat(oldfd, argv[2], newfd, argv[4])) {
		perror("renameat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (oldfd != AT_FDCWD)
		if (close(oldfd))
			perror("close");

	if (newfd != AT_FDCWD)
		if (close(newfd))
			perror("close");

	return ret;
}
