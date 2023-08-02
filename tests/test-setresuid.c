/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#ifndef __NetBSD__
int main(int argc, char * const argv[])
{
	uid_t ruid, euid, suid;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	ruid = strtoul(argv[1], NULL, 0);
	euid = strtoul(argv[2], NULL, 0);
	suid = strtoul(argv[3], NULL, 0);

	if (setresuid(ruid, euid, suid) == -1) {
		perror("setresuid");
		return EXIT_FAILURE;
	}

	printf("%s:%s:%s\n", getenv("IAMROOT_UID") ?: "",
			     getenv("IAMROOT_EUID") ?: "",
			     getenv("IAMROOT_SUID") ?: "");

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
