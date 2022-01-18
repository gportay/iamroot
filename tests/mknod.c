/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sysmacros.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

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
