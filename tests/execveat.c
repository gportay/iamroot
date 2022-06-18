/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <fcntl.h>
#include <unistd.h>

#ifdef __GLIBC__
#if !__GLIBC_PREREQ(2,34)
int execveat(int fd, const char *path, char * const argv[], char * const envp[], int flags)
{
	(void)fd;
	(void)flags;
	return execve(path, argv, envp);
}
#endif
#endif

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
