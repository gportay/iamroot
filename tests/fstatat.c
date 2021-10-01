/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	int fd = AT_FDCWD, flags = 0, ret = EXIT_FAILURE;
	struct stat statbuf;

	if (argc < 5) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 6) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc == 3)
		flags = strtoul(argv[3], NULL, 0);

	if (__strncmp(argv[1], "-") != 0) {
		fd = open(argv[1], O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (fstatat(fd, argv[2], &statbuf, flags)) {
		perror("fstatat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
