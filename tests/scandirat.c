/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <dirent.h>

#define __strncmp(s1, s2) strncmp(s1, s2, sizeof(s2)-1)

int main(int argc, char * const argv[])
{
	int n, fd = AT_FDCWD, ret = EXIT_FAILURE;
	struct dirent **namelist;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (__strncmp(argv[1], "-") != 0) {
		fd = open(".", O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	n = scandirat(fd, argv[2], &namelist, NULL, alphasort);
	if (n == -1) {
		perror("scandirat");
		goto exit;
	}

	while (n--)
		free(namelist[n]);
	free(namelist);

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
