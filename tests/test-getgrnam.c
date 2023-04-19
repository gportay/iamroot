/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <grp.h>

int main(int argc, char * const argv[])
{
	struct group *grp;
	char **n;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	grp = getgrnam(argv[1]);
	if (!grp) {
		perror("getgrnam");
		return EXIT_FAILURE;
	}

	printf("%s:%s:%u", grp->gr_name, grp->gr_passwd, grp->gr_gid);
	n = grp->gr_mem;
	while (*n)
		printf(":%s", *n++);
	printf("\n");

	return EXIT_SUCCESS;
}
