/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef __linux__
#include <sys/sysmacros.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __linux__
extern int __xmknod(int, const char *, mode_t, dev_t *);
#endif

int main(int argc, char * const argv[])
{
	mode_t mode;
	dev_t dev;
	int ver;

	if (argc < 6) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 6) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	ver = strtoul(argv[1], NULL, 0);
	mode = strtoul(argv[3], NULL, 0);
	dev = makedev(strtoul(argv[4], NULL, 0), strtoul(argv[5], NULL, 0));

#ifdef __linux__
	if (__xmknod(ver, argv[2], mode, &dev)) {
		perror("__xmknod");
		return EXIT_FAILURE;
	}
#else
	if (mknod(argv[2], mode, dev)) {
		perror("mknod");
		return EXIT_FAILURE;
	}
#endif

	return EXIT_SUCCESS;
}
