/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <shadow.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_ulckpwdf()
{
	int (*sym)();

	sym = dlsym(RTLD_NEXT, "ulckpwdf");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym();
}

int ulckpwdf()
{
	fprintf(stderr, "Warning: %s: %s\n", __func__, strerror(ENOTSUP));
	return next_ulckpwdf();
}
