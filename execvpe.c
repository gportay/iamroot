/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "path_resolution.h"

int execvpe(const char *file, char * const argv[], char * const envp[])
{
	int (*realsym)(const char *, char * const [], char * const []);
	const char *real_file;
	char buf[PATH_MAX];

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(file: '%s', argv: '%s'... , envp: '%s'...)\n",
				__func__, file, argv[0], envp[0]);

	real_file = path_resolution(file, buf, sizeof(buf));
	if (!real_file) {
		perror("path_resolution");
		return -1;
	}

	if (getenv("IAMROOT_DEBUG") && strcmp(file, real_file))
		fprintf(stderr, "%s(real_file: '%s', argv: '%s'... , envp: '%s'...)\n",
				__func__, real_file, argv[0], envp[0]);

	realsym = dlsym(RTLD_NEXT, __func__);
	return realsym(real_file, argv, envp);
}
