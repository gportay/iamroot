/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/mount.h>

int main(int argc, char * const argv[])
{
	if (argc < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

#ifdef __linux__
	if (umount(argv[1])) {
		perror("umount");
		return EXIT_FAILURE;
	}
#else
	(void)argv;
#endif

	return EXIT_SUCCESS;
}
