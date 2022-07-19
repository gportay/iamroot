/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "iamroot.h"

#ifdef __GLIBC__
int main(int argc, char * const argv[])
{
	int fd = AT_FDCWD, flags = 0, ret = EXIT_FAILURE;
	struct statx statxbuf;
	unsigned int mask;

	if (argc < 5) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 6) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	flags = strtoul(argv[3], NULL, 0);
	mask = strtoul(argv[3], NULL, 0);

	if (__strncmp(argv[1], "-") != 0) {
		fd = open(".", O_RDONLY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (statx(fd, argv[2], flags, mask, &statxbuf)) {
		perror("statx");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
