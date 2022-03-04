/*
 * Copyright 2021-2022 Gaël PORTAY
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
 * Slolen from musl (src/internal/syscall_ret.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static long __syscall_ret(unsigned long r)
{
	if (r > -4096UL) {
		errno = -r;
		return -1;
	}
	return r;
}

/*
 * Slolen from musl (src/process/fexecve.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
static int __fexecve(int fd, char * const argv[], char * const envp[])
{
	int r = __syscall(SYS_execveat, fd, "", argv, envp, AT_EMPTY_PATH);
	if (r != -ENOSYS) return __syscall_ret(r);
	char buf[15 + 3*sizeof(int)];
	__procfdname(buf, fd);
	execve(buf, argv, envp);
	if (errno == ENOENT) errno = EBADF;
	return -1;
}

int fexecve(int fd, char * const argv[], char * const envp[])
{
	__debug("%s(fd: %i, argv: { '%s', '%s', ... }, envp: %p)\n",
		__func__, fd, argv[0], argv[1], envp);

	return __fexecve(fd, argv, envp);
}
#endif
