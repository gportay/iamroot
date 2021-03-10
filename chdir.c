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

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int chrootdir(const char *);

int next_chdir(const char *path)
{
	int (*sym)(const char *);

	sym = dlsym(RTLD_NEXT, "chdir");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path);
}

int chdir(const char *path)
{
	const char *real_path;
	char buf[PATH_MAX];
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = next_chdir(real_path);
	if (ret) {
		perror("chdir");
		goto exit;
	}

	if (chrootdir(real_path))
		perror("chrootdir");

exit:
	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return ret;
}
