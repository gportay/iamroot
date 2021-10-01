/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

int main(int argc, char * const argv[])
{
	int fd = -1, ret = EXIT_FAILURE;
	int mode = F_OK;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (argc == 3)
		mode = strtoul(argv[4], NULL, 0);

	fd = access(argv[1], mode);
	if (fd == -1) {
		perror("access");
		goto exit;
	}

	ret = EXIT_SUCCESS;

exit:
	if (fd == -1)
		if (close(fd))
			perror("close");

	return ret;
}
