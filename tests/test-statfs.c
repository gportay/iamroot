/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
#include <sys/statfs.h>
#endif

#if defined __FreeBSD__ || defined __OpenBSD__
#include <sys/param.h>
#include <sys/mount.h>
#endif

#ifndef __NetBSD__
int main(int argc, char * const argv[])
{
	struct statfs buf;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (statfs(argv[1], &buf)) {
		perror("statfs");
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
