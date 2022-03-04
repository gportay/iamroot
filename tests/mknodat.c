/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __linux__
#include <sys/sysmacros.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	int fd = AT_FDCWD, ret = EXIT_FAILURE;
	mode_t mode;
	dev_t dev;

	if (argc < 6) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 6) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	mode = strtoul(argv[3], NULL, 0);
	dev = makedev(strtoul(argv[4], NULL, 0), strtoul(argv[5], NULL, 0));

	if (__strncmp(argv[1], "-") != 0) {
		fd = open(".", O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (mknodat(fd, argv[2], mode, dev)) {
		perror("mknodat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
