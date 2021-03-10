/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	uid_t owner;
	gid_t group;
	int fd;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	owner = strtoul(argv[2], NULL, 0);
	group = strtoul(argv[3], NULL, 0);

	fd = open(argv[1], O_CREAT, 0644);
	if (fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	if (close(fd))
		perror("close");

	if (chown(argv[1], owner, group)) {
		perror("chown");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
