/*
 * Copyright 2020 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "path_resolution.h"

int execve(const char *path, char *const argv[], char *const envp[])
{
	int (*realsym)(const char *, char *const [], char *const []);
	const char *real_path;
	char buf[PATH_MAX];

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s', argv: '%s'... , envp: '%s'...)\n",
				__func__, path, argv[0], envp[0]);

	real_path = path_resolution(path, buf, sizeof(buf));
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	realsym = dlsym(RTLD_NEXT, __func__);
	return realsym(real_path, argv, envp);
}
