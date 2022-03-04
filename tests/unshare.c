/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sched.h>

#ifdef __linux__
int main(int argc, char * const argv[])
{
	if (unshare(0) == -1) {
		perror("unshare");
		return EXIT_FAILURE;
	}

	if (argc < 2)
		return EXIT_SUCCESS;

	execvp(argv[1], &argv[1]);
	perror("execvp");
	return EXIT_FAILURE;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
