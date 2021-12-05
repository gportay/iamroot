/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	int fd = AT_FDCWD, ret = EXIT_FAILURE;
	char buf[PATH_MAX];
	ssize_t siz;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (__strncmp(argv[2], "-") != 0) {
		fd = open(".", O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	siz = readlinkat(fd, argv[2], buf, sizeof(buf)-1);
	if (siz == -1) {
		perror("readlinkat");
		goto exit;
	}
	buf[siz] = 0;

	printf("%s\n", buf);

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
