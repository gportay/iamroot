/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	int dfd = AT_FDCWD, ret = EXIT_FAILURE;
	char buf[PATH_MAX];
	ssize_t siz;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (!__strneq(argv[2], "-")) {
		dfd = open(".", O_DIRECTORY);
		if (dfd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	siz = readlinkat(dfd, argv[2], buf, sizeof(buf)-1);
	if (siz == -1) {
		perror("readlinkat");
		goto exit;
	}
	buf[siz] = 0;

	printf("%s\n", buf);

	ret = EXIT_SUCCESS;

exit:
	if (dfd != AT_FDCWD)
		if (close(dfd))
			perror("close");

	return ret;
}
