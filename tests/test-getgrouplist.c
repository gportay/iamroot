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
#ifdef __linux__
#include <linux/limits.h>
#endif
#if defined __FreeBSD__ || defined __OpenBSD__ || defined __NetBSD__
#include <limits.h>
#include <sys/syslimits.h>
#endif

#include <grp.h>

int main(int argc, char * const argv[])
{
	gid_t gid, groups[NGROUPS_MAX];
	int i, siz, len = NGROUPS_MAX;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	gid = strtoul(argv[2], NULL, 0);
	if (argc > 3)
		len = strtoul(argv[3], NULL, 0);

	siz = getgrouplist(argv[1], gid, groups, &len);
	if (siz == -1 && errno) {
		perror("getgrouplist");
		return EXIT_FAILURE;
	}

	printf("%i", len);
	for (i = 0; i < siz; i++)
		printf(" %u", groups[i]);
	printf("\n");

	return EXIT_SUCCESS;
}
