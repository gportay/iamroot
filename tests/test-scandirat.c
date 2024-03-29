/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <dirent.h>

#include "iamroot.h"

#ifdef  __USE_GNU
int main(int argc, char * const argv[])
{
	int n, fd = AT_FDCWD, ret = EXIT_FAILURE;
	struct dirent **namelist;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (!__strneq(argv[1], "-")) {
		fd = open(".", O_DIRECTORY);
		if (fd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	n = scandirat(fd, argv[2], &namelist, NULL, alphasort);
	if (n == -1) {
		perror("scandirat");
		goto exit;
	}

	while (n--)
		free(namelist[n]);
	free(namelist);

	ret = EXIT_SUCCESS;

exit:
	if (fd != AT_FDCWD)
		if (close(fd))
			perror("close");

	return ret;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
