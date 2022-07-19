/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	int fd = AT_FDCWD, flags = 0, ret = EXIT_FAILURE;
	uid_t owner;
	gid_t group;

	if (argc < 5) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 6) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	owner = strtoul(argv[3], NULL, 0);
	group = strtoul(argv[4], NULL, 0);
	if (argc == 6)
		flags = strtoul(argv[5], NULL, 0);

	if (__strncmp(argv[1], "-") != 0) {
		fd = open(argv[1], O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (fchownat(fd, argv[2], owner, group, flags)) {
		perror("fchownat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
