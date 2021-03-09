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

int main(int argc, char * const argv[])
{
	int fd, ret = EXIT_FAILURE;
	uid_t owner;
	gid_t group;

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

	if (fchown(fd, owner, group)) {
		perror("fchown");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (close(fd))
		perror("close");

	return ret;
}
