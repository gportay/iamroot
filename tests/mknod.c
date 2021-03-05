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

int mknod(const char *pathname, mode_t mode, dev_t dev);

int main(int argc, char * const argv[])
{
	mode_t mode;
	dev_t dev;

	if (argc < 5) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 5) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	mode = strtoul(argv[2], NULL, 0);
	dev = makedev(strtoul(argv[3], NULL, 0), strtoul(argv[4], NULL, 0));

	if (mknod(argv[1], mode, dev)) {
		perror("mknod");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
