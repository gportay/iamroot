/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	char buf[PATH_MAX];
	ssize_t siz;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	siz = readlink(argv[1], buf, sizeof(buf)-1);
	if (siz == -1) {
		perror("readlink");
		return EXIT_FAILURE;
	}
	buf[siz] = 0;

	printf("%s\n", buf);

	return EXIT_SUCCESS;
}
