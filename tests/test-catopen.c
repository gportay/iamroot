/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <nl_types.h>

int main(int argc, char * const argv[])
{
	nl_catd cd;
	int flag;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	flag = strtoul(argv[2], NULL, 0);

	cd = catopen(argv[1], flag);
	if (cd == (nl_catd)-1) {
		perror("catopen");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
