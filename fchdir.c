/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern char *next_getcwd(char *, size_t);
extern int chrootdir(const char *);

int next_fchdir(int fd)
{
	int (*sym)(int);

	sym = dlsym(RTLD_NEXT, "fchdir");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd);
}

int fchdir(int fd)
{
	char cwd[PATH_MAX];
	int ret;

	ret = next_fchdir(fd);
	if (ret) {
		perror("fchdir");
		goto exit;
	}

	if (next_getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("getcwd");
		goto exit;
	}

	if (chrootdir(cwd))
		perror("chrootdir");

exit:
	__fprintf(stderr, "%s(fd: %i '%s')\n", __func__, fd, cwd);

	return ret;
}
