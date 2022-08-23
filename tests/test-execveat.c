/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <paths.h>

#include <fcntl.h>
#include <unistd.h>

#ifdef __GLIBC__
#if !__GLIBC_PREREQ(2,34)
int execveat(int dfd, const char *path, char * const argv[],
	     char * const envp[], int atflags)
{
	(void)dfd;
	(void)atflags;

	return execve(path, argv, envp);
}
#endif
#endif

#ifdef __FreeBSD__
extern char **environ;
#endif

int main(void)
{
	char * const argv[] = { "-sh", "-c", "echo \"$@\"", "sh", "one", "two",
				"three", NULL };

#ifdef __linux__
	execveat(AT_FDCWD, _PATH_BSHELL, argv, environ, AT_EMPTY_PATH);
	perror("execveat");
#else
	execve(_PATH_BSHELL, argv, environ);
	perror("execve");
#endif
	_exit(127);
}
