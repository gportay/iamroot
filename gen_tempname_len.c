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

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int next_gen_tempname_len(char *path, int suffixlen, int flags, int kind,
			  size_t x_suffix_len)
{
	int (*sym)(char *, int, int, int, size_t);

	sym = dlsym(RTLD_NEXT, "gen_tempname_len");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, suffixlen, flags, kind, x_suffix_len);
}

int gen_tempname_len(char *path, int suffixlen, int flags, int kind,
		     size_t x_suffix_len)
{
	char *real_path;
	char buf[PATH_MAX];

	real_path = (char *)path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_gen_tempname_len(real_path, suffixlen, flags, kind,
				     x_suffix_len);
}
