/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __linux__
#include <sys/sysmacros.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "iamroot.h"

#ifdef __linux__
extern int __xmknodat(int, int, const char *, mode_t, dev_t *);
#endif

int main(int argc, char * const argv[])
{
	int ver, fd = AT_FDCWD, ret = EXIT_FAILURE;
	mode_t mode;
	dev_t dev;

	if (argc < 7) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 7) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	ver = strtoul(argv[1], NULL, 0);
	mode = strtoul(argv[4], NULL, 0);
	dev = makedev(strtoul(argv[5], NULL, 0), strtoul(argv[6], NULL, 0));

	if (__strncmp(argv[2], "-") != 0) {
		fd = open(".", O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

#ifdef __linux__
	if (__xmknodat(ver, fd, argv[3], mode, &dev)) {
		perror("__xmknodat");
		goto exit;
	}
#else
	if (mknodat(fd, argv[3], mode, dev)) {
		perror("mknodat");
		goto exit;
	}
#endif

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
