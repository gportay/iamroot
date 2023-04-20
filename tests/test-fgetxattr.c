/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#ifdef __linux__
#include <sys/xattr.h>

int main(int argc, char * const argv[])
{
	char buf[BUFSIZ];
	ssize_t len;
	int fd;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	len = fgetxattr(fd, argv[2], buf, sizeof(buf));
	if (len == -1) {
		perror("fgetxattr");
		return EXIT_FAILURE;
	}
	buf[len] = 0; /* NULL-terminated */

	printf("%s\n", buf);

	return EXIT_SUCCESS;
}
#else
int main(void)
{
	return EXIT_SUCCESS;
}
#endif
