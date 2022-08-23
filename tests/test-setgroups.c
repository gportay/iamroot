/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <grp.h>

int main(int argc, char * const argv[])
{
	size_t listsize;
	int siz;

	listsize = argc-1; /* argv0 */

	if (listsize == 0) {
		siz = setgroups(0, NULL);
		if (siz == -1) {
			perror("setgroups");
			return EXIT_FAILURE;
		}
	}

	if (listsize > 0) {
		gid_t list[listsize];
		size_t i;

		for (i = 0; i < listsize; i++)
			list[i] = strtoul(argv[i+1], NULL, 0); /* argv0 */

		siz = setgroups(listsize, list);
		if (siz == -1) {
			perror("setgroups");
			return EXIT_FAILURE;
		}
	}

	printf("%s\n", getenv("IAMROOT_GROUPS") ?: "");

	return EXIT_FAILURE;
}
