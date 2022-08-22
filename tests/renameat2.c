/*
 * Copyright 2021-2022 Gaël PORTAY
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
	int oldfd = AT_FDCWD, newfd = AT_FDCWD, ret = EXIT_FAILURE;
	unsigned int flags = 0;

	if (argc < 6) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 6) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	flags = strtoul(argv[5], NULL, 0);

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

#ifdef __linux__
	if (renameat2(oldfd, argv[2], newfd, argv[4], flags)) {
		perror("renameat2");
		goto exit;
	}
#else
	(void)flags;
	if (renameat(oldfd, argv[2], newfd, argv[4])) {
		perror("renameat");
		goto exit;
	}
#endif

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
