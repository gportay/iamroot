/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

int main(int argc, char * const argv[])
{
	char buf[L_tmpnam];

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	memset(buf, 0, sizeof(buf));
	strncpy(buf, argv[1], sizeof(buf)-1);

	if (tmpnam(buf) == NULL) {
		perror("tmpnam");
		return EXIT_FAILURE;
	}

	printf("%s\n", buf);

	return EXIT_SUCCESS;
}
