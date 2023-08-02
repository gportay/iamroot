/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <paths.h>
#include <fcntl.h>

#include <unistd.h>

#if defined __FreeBSD__ || defined __OpenBSD__
extern char **environ;
#endif

int main(void)
{
	char * const argv[] = { "-sh", "-c", "echo \"$@\"", "sh", "one", "two",
				"three", NULL };
#if defined __linux__ || __FreeBSD__
	int fd;

	fd = open(_PATH_BSHELL, O_RDONLY);
	if (fd == -1)
		_exit(127);

	fexecve(fd, argv, environ);
	perror("fexecve");
#else
	execve(_PATH_BSHELL, argv, environ);
	perror("execve");
#endif
	_exit(127);
}
