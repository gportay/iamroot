/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

#ifndef __GLIBC_PREREQ
#define __GLIBC_PREREQ(maj,min) 0
#endif

#if (defined __GLIBC__ && __GLIBC_PREREQ(2,34)) || defined __FreeBSD__
int main(int argc, char * const argv[])
{
	unsigned int first, last;
	int flags;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	first = strtoul(argv[1], NULL, 0);
	last = strtoul(argv[2], NULL, 0);
	flags = strtoul(argv[3], NULL, 0);

	if (close_range(first, last, flags)) {
		perror("close_range");
		return EXIT_FAILURE;
	}

	fprintf(stderr, "%s\n", "stderr");
	fprintf(stdout, "%s\n", "stdout");

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
