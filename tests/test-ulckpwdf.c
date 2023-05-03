/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
#include <shadow.h>

int main(int argc, char * const argv[])
{
	(void)argv;

	if (argc < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 1) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	ulckpwdf();

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
