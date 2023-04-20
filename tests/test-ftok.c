/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/ipc.h>

int main(int argc, char * const argv[])
{
	int proj_id;
	key_t key;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	proj_id = strtoul(argv[2], NULL, 0);

	key = ftok(argv[1], proj_id);
	if (key == (key_t)-1) {
		perror("ftok");
		return EXIT_FAILURE;
	}

	printf("%x\n", (int)key);

	return EXIT_SUCCESS;
}
