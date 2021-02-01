/*
 * Copyright 2021 Gaël PORTAY
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

int chdir(const char *path)
{
	int (*realsym)(const char *);
	const char *real_path;
	char buf[PATH_MAX];

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s')\n", __func__, path);

	real_path = path_resolution(path, buf, sizeof(buf));
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s'): real_path: '%s'\n", __func__, path, real_path);

	realsym = dlsym(RTLD_NEXT, __func__);
	return realsym(real_path);
}
