/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <paths.h>

#include <unistd.h>

#ifdef __FreeBSD__
extern char **environ;
#endif

int main(void)
{
	char * const argv[] = { "-sh", "-c", "echo \"$@\"", "sh", "one", "two",
				"three", NULL };

	execve(_PATH_BSHELL, argv, environ);
	perror("execve");
	_exit(127);
}
