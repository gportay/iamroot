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
#include <limits.h>
#endif

#include <sys/types.h>
#include <grp.h>

#ifdef __linux__
int main(int argc, char * const argv[])
{
	char *groups[NGROUPS_MAX+1] = { NULL }; /* NULL-terminated */
	struct group grp;
	int i;
	(void)argv;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > (4 + NGROUPS_MAX)) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	grp.gr_name = argv[1];
	grp.gr_passwd = argv[2];
	grp.gr_gid = strtoul(argv[3], NULL, 0);
	for (i = 0; i < argc-4; i++)
		groups[i] = argv[4+i];
	grp.gr_mem = groups;

	if (putgrent(&grp, stdout)) {
		perror("putgrent");
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
