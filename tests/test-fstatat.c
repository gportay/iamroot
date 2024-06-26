/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	int dfd = AT_FDCWD, flags = 0, ret = EXIT_FAILURE;
	struct stat statbuf;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc > 3)
		flags = strtoul(argv[3], NULL, 0);

	if (!__strneq(argv[1], "-")) {
		dfd = open(argv[1], O_DIRECTORY);
		if (dfd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (fstatat(dfd, argv[2], &statbuf, flags)) {
		perror("fstatat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (dfd != AT_FDCWD)
		if (close(dfd))
			perror("close");

	return ret;
}
