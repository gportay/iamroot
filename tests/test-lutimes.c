/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>

int main(int argc, char * const argv[])
{
	struct timeval times[2] = { 0 };

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	times[0].tv_sec = strtoul(argv[2], NULL, 0);
	times[1].tv_sec = strtoul(argv[3], NULL, 0);

	if (lutimes(argv[1], times)) {
		perror("lutimes");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
