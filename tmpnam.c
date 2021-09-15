/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdio.h>

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__                                                                
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

#include <features.h>

__attribute__((visibility("hidden")))
char *next_tmpnam(char *path)
{
	char *(*sym)(char *);

	sym = dlsym(RTLD_NEXT, "tmpnam");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(path);
}

#if __GLIBC_PREREQ(2, 34)
char *tmpnam(char path[L_tmpnam])
#else
char *tmpnam(char *path)
#endif
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution((char *)path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_tmpnam(real_path);
}

#endif
