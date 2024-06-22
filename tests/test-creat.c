/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char * const argv[])
{
	mode_t mode;
	int fd;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	mode = strtoul(argv[2], NULL, 0);

	fd = creat(argv[1], mode);
	if (fd == -1) {
		perror("creat");
		return EXIT_FAILURE;
	}

	if (close(fd))
		perror("close");

	return EXIT_SUCCESS;
}
