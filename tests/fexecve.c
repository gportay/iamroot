/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <fcntl.h>

#include <unistd.h>

int main(void)
{
	char * const argv[] = { "-sh", "-c", "echo \"$@\"", "sh", "one", "two",
				"three", NULL };
	int fd;

	fd = open("/bin/sh", O_RDONLY);
	if (fd == -1)
		_exit(127);

	fexecve(fd, argv, environ);
	_exit(127);
}
