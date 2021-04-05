/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next___open(const char *path, int flags, mode_t mode)
{
	int (*sym)(const char *, int, ...);

	sym = dlsym(RTLD_NEXT, "__open");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, flags, mode);
}

int __open(const char *path, int flags, ...)
{
	const char *real_path;
	char buf[PATH_MAX];
	mode_t mode = 0;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next___open(real_path, flags, mode);
}
#endif
