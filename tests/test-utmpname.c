/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __linux__
#include <utmp.h>

int main(int argc, char * const argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (utmpname(argv[1]) == -1) {
		perror("utmpname");
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
