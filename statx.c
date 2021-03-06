/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);
#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int rootstatx(int, const char *, int, unsigned int, struct statx *);

__attribute__((visibility("hidden")))
int next_statx(int fd, const char *path, int flags, unsigned int mask,
	       struct statx *statxbuf)
{
	int (*sym)(int, const char *, int, unsigned int, struct statx *);

	sym = dlsym(RTLD_NEXT, "statx");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, path, flags, mask, statxbuf);
}

int statx(int fd, const char *path, int flags, unsigned int mask,
	  struct statx *statxbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__fprintf(stderr, "%s(fd: %i, path: '%s' -> '%s', ...)\n", __func__, fd,
			  path, real_path);

	return rootstatx(fd, real_path, flags, mask, statxbuf);
}
#endif
