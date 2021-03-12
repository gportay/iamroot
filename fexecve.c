/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <errno.h>
#include <sys/syscall.h>
#include <fcntl.h>

#include <unistd.h>

#define __syscall syscall

/* Slolen from musl (src/internal/syscall_ret.c) */
long __syscall_ret(unsigned long r)
{
	if (r > -4096UL) {
		errno = -r;
		return -1;
	}
	return r;
}

/* Slolen from musl (src/internal/procfdname.c) */
__attribute__((visibility("hidden")))
void __procfdname(char *buf, unsigned fd)
{
	unsigned i, j;
	for (i=0; (buf[i] = "/proc/self/fd/"[i]); i++);
	if (!fd) {
		buf[i] = '0';
		buf[i+1] = 0;
		return;
	}
	for (j=fd; j; j/=10, i++);
	buf[i] = 0;
	for (; fd; fd/=10) buf[--i] = '0' + fd%10;
}

/* Slolen from musl (src/process/fexecve.c) */
int fexecve(int fd, char * const argv[], char * const envp[])
{
	int r = __syscall(SYS_execveat, fd, "", argv, envp, AT_EMPTY_PATH);
	if (r != -ENOSYS) return __syscall_ret(r);
	char buf[15 + 3*sizeof(int)];
	__procfdname(buf, fd);
	execve(buf, argv, envp);
	if (errno == ENOENT) errno = EBADF;
	return -1;
}
