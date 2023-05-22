/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#ifdef __GLIBC__
#include <stdint.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/fanotify.h>

#include "iamroot.h"

#ifdef __GLIBC__
static int (*sym)(int, unsigned int, uint64_t, int, const char *);
#else
static int (*sym)(int, unsigned int, unsigned long long, int, const char *);
#endif

__attribute__((visibility("hidden")))
#ifdef __GLIBC__
int next_fanotify_mark(int fanotify_fd, unsigned int flags, uint64_t mask,
		       int dfd, const char *path)
#else
int next_fanotify_mark(int fanotify_fd, unsigned int flags, unsigned long mask,
		       int dfd, const char *path)
#endif
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fanotify_mark");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(fanotify_fd, flags, mask, dfd, path);
}

#ifdef __GLIBC__
int fanotify_mark(int fanotify_fd, unsigned int flags, uint64_t mask, int dfd,
		  const char *path)
#else
int fanotify_mark(int fanotify_fd, unsigned int flags, unsigned long long mask,
		  int dfd, const char *path)
#endif
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_fanotify_mark(fanotify_fd, flags, mask, dfd, path);

	__debug("%s(..., dfd: %i <-> '%s', path: '%s') -> %i\n", __func__, dfd,
		__fpath(dfd), path, ret);

	return ret;
}
#endif
