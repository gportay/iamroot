/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

extern char *path_resolution(const char *path, char *buf, size_t bufsize,
			     int flags);

int main(int argc, char * const argv[])
{
	char buf[PATH_MAX];

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (path_resolution(argv[1], buf, sizeof(buf), 0) == NULL) {
		perror("path_resolution");
		return EXIT_FAILURE;
	}

	printf("%s\n", buf);

	return EXIT_SUCCESS;
}
