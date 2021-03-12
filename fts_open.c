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

#ifdef __GLIBC__
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
FTS *next_fts_open(char * const *path_argv, int options,
		   int (*compar)(const FTSENT **, const FTSENT **))
{
	FTS *(*sym)(char * const *, int,
		    int (*)(const FTSENT **, const FTSENT **));

	sym = dlsym(RTLD_NEXT, "fts_open");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(path_argv, options, compar);
}

FTS *fts_open(char * const *path_argv, int options,
	      int (*compar)(const FTSENT **, const FTSENT **))
{
	fprintf(stderr, "Warning: %s: %s\n", __func__, strerror(ENOTSUP));
	return next_fts_open(path_argv, options, compar);
}
#endif
