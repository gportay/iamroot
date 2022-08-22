/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <unistd.h>
#include <fcntl.h>

#include "iamroot.h"

int main(int argc, char * const argv[])
{
	int dfd = AT_FDCWD, ret = EXIT_FAILURE;
	char buf[PATH_MAX];
	int length;
	int flags;

	if (argc < 5) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 5) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}


	memset(buf, 0, sizeof(buf));
	strncpy(buf, argv[2], sizeof(buf)-1);

	length = strtol(argv[3], NULL, 0);
	flags = strtol(argv[4], NULL, 0);

#ifdef __FreeBSD__
	if (__strncmp(argv[1], "-") != 0) {
		dfd = open(".", O_DIRECTORY);
		if (dfd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	if (mkostempsat(dfd, buf, length, flags) == -1) {
		perror("mkostempsat");
		goto exit;
	}
#else
	if (mkostemps(buf, length, flags) == -1) {
		perror("mkostemps");
		goto exit;
	}
#endif

	printf("%s\n", buf);

	ret = EXIT_SUCCESS;

exit:
	if (dfd != AT_FDCWD)
		if (close(dfd))
			perror("close");

	return ret;
}
