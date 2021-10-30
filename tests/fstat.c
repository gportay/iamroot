/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>

int main(int argc, char * const argv[])
{
	int fd, ret = EXIT_FAILURE;
	struct stat statbuf;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	if (fstat(fd, &statbuf)) {
		perror("fstat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (close(fd))
		perror("close");

	return ret;
}
