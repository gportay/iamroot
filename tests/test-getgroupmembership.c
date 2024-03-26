/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#ifdef __NetBSD__
#include <sys/syslimits.h>
#endif

#include <grp.h>

#ifdef __NetBSD__
int main(int argc, char * const argv[])
{
	gid_t gid, groups[NGROUPS_MAX];
	int i, ret, len, maxlen = NGROUPS_MAX;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	gid = strtoul(argv[2], NULL, 0);
	if (argc > 3)
		maxlen = strtoul(argv[3], NULL, 0);

	ret = getgroupmembership(argv[1], gid, groups, maxlen, &len);
	if (ret == -1) {
		perror("getgroupmembership");
		return EXIT_FAILURE;
	}

	printf("%i", len);
	for (i = 0; i < len; i++)
		printf(" %u", groups[i]);
	printf("\n");

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
