/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <grp.h>

#ifdef __linux__
int main(int argc, char * const argv[])
{
	struct group *grp;
	(void)argv;

	if (argc < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 1) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	while ((grp = fgetgrent(stdin))) {
		char **n;

		printf("%s:%s:%u", grp->gr_name, grp->gr_passwd, grp->gr_gid);
		n = grp->gr_mem;
		while (*n)
			printf(":%s", *n++);
		printf("\n");
	}
	if (errno && errno != ENOENT) {
		perror("fgetgrent");
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
