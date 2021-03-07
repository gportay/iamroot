/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/sysmacros.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)

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
		fd = open(".", 0);
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
