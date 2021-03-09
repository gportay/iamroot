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

#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)

int main(int argc, char * const argv[])
{
	int flags = 0, fd = AT_FDCWD, fd2 = -1, ret = EXIT_FAILURE;
	mode_t mode;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 5) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc > 3)
		flags = strtoul(argv[3], NULL, 0);

	if (argc > 4)
		mode = strtoul(argv[4], NULL, 0);

	if (__strncmp(argv[1], "-") != 0) {
		fd = open(".", O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	fd2 = openat(fd, argv[2], mode, flags);
	if (fd2 == -1) {
		perror("openat");
		goto exit;
	}

	if (fd2 != -1)
		if (close(fd2))
			perror("close");

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
