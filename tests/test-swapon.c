/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
#include <sys/swap.h>

int main(int argc, char * const argv[])
{
	int flags;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	flags = strtol(argv[2], NULL, 0);

	if (swapon(argv[1], flags) == -1){
		perror("swapon");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
