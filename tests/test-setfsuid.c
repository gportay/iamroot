/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __linux__
#include <sys/fsuid.h>

int main(int argc, char * const argv[])
{
	uid_t uid;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	uid = strtoul(argv[1], NULL, 0);

	if (setfsuid(uid) == -1) {
		perror("setfsuid");
		return EXIT_FAILURE;
	}

	printf("%s\n", getenv("IAMROOT_FSUID") ?: "");

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
