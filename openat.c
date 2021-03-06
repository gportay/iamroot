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

#include <fcntl.h>

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_openat(int fd, const char *path, int flags, mode_t mode)
{
	int (*sym)(int, const char *, int, ...);

	sym = dlsym(RTLD_NEXT, "openat");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, path, flags, mode);
}

int openat(int fd, const char *path, int flags, ...)
{
	char buf[PATH_MAX];
	char *real_path;
	mode_t mode = 0;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	__fprintf(stderr, "%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd,
			  path, real_path);

	return next_openat(fd, real_path, flags, mode);
}
