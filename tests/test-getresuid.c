/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#ifndef __NetBSD__
int main(int argc, char * const argv[])
{
	uid_t ruid, euid, suid;
	(void)argv;

	if (argc < 1) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 1) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (getresuid(&ruid, &euid, &suid)) {
		perror("getresuid");
		return EXIT_FAILURE;
	}

	printf("%u:%u:%u\n", ruid, euid, suid);

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
