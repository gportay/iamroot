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

__attribute__((visibility("hidden")))
#ifdef __GLIBC__
int next_fanotify_mark(int fanotify_fd, unsigned int flags, uint64_t mask,
		       int dfd, const char *path)
#else
int next_fanotify_mark(int fanotify_fd, unsigned int flags, unsigned long mask,
		       int dfd, const char *path)
#endif
{
#ifdef __GLIBC__
	int (*sym)(int, unsigned int, uint64_t, int, const char *);
#else
	int (*sym)(int, unsigned int, unsigned long long, int, const char *);
#endif
	int ret;

	sym = dlsym(RTLD_NEXT, "fanotify_mark");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(fanotify_fd, flags, mask, dfd, path);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
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

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(..., dfd: %i <-> '%s', path: '%s')\n", __func__, dfd,
		__fpath(dfd), path);

	return next_fanotify_mark(fanotify_fd, flags, mask, dfd, path);
}
#endif
