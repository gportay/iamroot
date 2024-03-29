/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <errno.h>
#include <sys/syscall.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

#define __syscall syscall

/*
 * Stolen from musl (src/process/fexecve.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static int __fexecve(int fd, char * const argv[], char * const envp[])
{
#ifdef SYS_execveat
	int r = __syscall(SYS_execveat, fd, "", argv, envp, AT_EMPTY_PATH);
	if (r != -ENOSYS) return __syscall_ret(r);
#endif
	char buf[15 + 3*sizeof(int)];
	__procfdname(buf, fd);
	execve(buf, argv, envp);
	if (errno == ENOENT) errno = EBADF;
	return -1;
}

#undef __syscall

int fexecve(int fd, char * const argv[], char * const envp[])
{
	__debug("%s(fd: %i <-> '%s', argv: { '%s', '%s', ... }, envp: %p)\n",
		__func__, fd, __fpath(fd), argv[0], argv[1], envp);

	/* Forward to local function */
	return __fexecve(fd, argv, envp);
}
#endif
