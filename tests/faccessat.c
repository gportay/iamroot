/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)

int main(int argc, char * const argv[])
{
	int mode, flag, fd = AT_FDCWD, ret = EXIT_FAILURE;

	if (argc < 5) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 5) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	mode = strtoul(argv[3], NULL, 0);
	flag = strtoul(argv[4], NULL, 0);

	if (__strncmp(argv[1], "-") != 0) {
		fd = open(".", O_RDONLY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (faccessat(fd, argv[2], mode, flag)) {
		perror("faccessat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
