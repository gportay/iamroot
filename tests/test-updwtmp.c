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
	struct utmp ut;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	memset(&ut, 0, sizeof(ut));
	updwtmp(argv[1], &ut);

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
