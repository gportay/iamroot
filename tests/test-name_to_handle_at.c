/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>

#include "iamroot.h"

#ifdef __linux__
int main(int argc, char * const argv[])
{
	int id, atflags = 0, dfd = AT_FDCWD, ret = EXIT_FAILURE;
	unsigned char buf[MAX_HANDLE_SZ];
	struct file_handle *h;

	if (argc < 4) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 4) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	atflags = strtoul(argv[3], NULL, 0);

	if (!__strneq(argv[1], "-")) {
		dfd = open(".", O_DIRECTORY);
		if (dfd == -1) {
			perror("open");
			return EXIT_FAILURE;
		}
	}

	h = (struct file_handle *)buf;
	h->handle_bytes = sizeof(buf);
	if (name_to_handle_at(dfd, argv[2], h, &id, atflags) == -1) {
		perror("name_to_handle_at");
		goto exit;
	}

	printf("%i %u %i\n", id, h->handle_type, h->handle_bytes);

	ret = EXIT_SUCCESS;

exit:
	if (dfd != AT_FDCWD)
		if (close(dfd))
			perror("close");

	return ret;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
