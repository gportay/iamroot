/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	int len = 0;
	int siz;

	if (argc < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc > 1)
		len = strtoul(argv[1], NULL, 0);

	siz = getgroups(0, NULL);
	if (siz == -1) {
		perror("getgroups");
		return EXIT_FAILURE;
	}

	if (len == 0)
		len = siz;
	if (len > 0) {
		gid_t list[len];
		int i;

		siz = getgroups(len, list);
		if (siz == -1) {
			perror("getgroups");
			return EXIT_FAILURE;
		}

		printf("%i", siz);
		for (i = 0; i < siz; i++)
			printf(" %u", list[i]);
		printf("\n");

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}
