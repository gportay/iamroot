/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
#include <sys/xattr.h>

int main(int argc, char * const argv[])
{
	char buf[BUFSIZ];
	ssize_t len;
	int i;

	if (argc < 2) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc > 2) {
		fprintf(stderr, "Too many arguments\n");
		exit(EXIT_FAILURE);
	}

	len = llistxattr(argv[1], buf, sizeof(buf));
	if (len == -1) {
		perror("llistxattr");
		return EXIT_FAILURE;
	}

	for (i = 0; i <= len; i++)
		if (buf[i] == 0)
			buf[i] = ':';
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
