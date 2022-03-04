/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	char * const argv[] = { "-sh", "-c", "echo \"$@\"", "sh", "one", "two",
				"three", NULL };

#ifdef __linux__
	execveat(AT_FDCWD, "/bin/sh", argv, environ, AT_EMPTY_PATH);
#else
	execve("/bin/sh", argv, environ);
#endif
	_exit(127);
}
