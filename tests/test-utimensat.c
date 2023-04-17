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
	int atflags = 0, fd = AT_FDCWD, ret = EXIT_FAILURE;
	struct timespec times[2] = { 0 };

	if (argc < 5) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 6) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	times[0].tv_sec = strtoul(argv[3], NULL, 0);
	times[1].tv_sec = strtoul(argv[4], NULL, 0);
	if (argc == 6)
		atflags = strtoul(argv[5], NULL, 0);

	if (!__strneq(argv[1], "-")) {
		fd = open(".", O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (utimensat(fd, argv[2], times, atflags)) {
		perror("utimensat");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
