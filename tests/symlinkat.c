/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)

int main(int argc, char * const argv[])
{
	int fd = AT_FDCWD, ret = EXIT_FAILURE;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (__strncmp(argv[2], "-") != 0) {
		fd = open(".", O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (symlinkat(argv[1], fd, argv[3])) {
		perror("symlinkat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
