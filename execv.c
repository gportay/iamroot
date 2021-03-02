/*
 * Copyright 2020-2021 Gaël PORTAY
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

int execv(const char *path, char * const argv[])
{
	int (*realsym)(const char *, char * const []);
	const char *real_path;
	char buf[PATH_MAX];

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s', argv: '%s'...)\n",
				__func__, path, argv[0]);

	real_path = path_resolution(path, buf, sizeof(buf));
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	if (getenv("IAMROOT_DEBUG") && strcmp(path, real_path))
		fprintf(stderr, "%s(real_path: '%s', argv: '%s'...)\n",
				__func__, real_path, argv[0]);

	realsym = dlsym(RTLD_NEXT, __func__);
	return realsym(real_path, argv);
}
