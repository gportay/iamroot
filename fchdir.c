/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

extern int __fprintf(FILE *, const char *, ...);
extern char *next_getcwd(char *, size_t);

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
	char *root;
	int ret;

	ret = next_fchdir(fd);
	if (ret) {
		perror("fchdir");
		goto exit;
	}

	root = getenv("IAMROOT_ROOT");
	if (!root)
		goto exit;

	if (next_getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("getcwd");
		goto exit;
	}

	if (strstr(cwd, root) == NULL)
		unsetenv("IAMROOT_ROOT");

exit:
	__fprintf(stderr, "%s(fd: %i '%s'): IAMROOT_ROOT: '%s'\n", __func__,
			  fd, cwd, root);

	return ret;
}
