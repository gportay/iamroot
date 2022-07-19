/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/inotify.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_inotify_add_watch(int fd, const char *path, uint32_t mask)
{
	int (*sym)(int, const char *, uint32_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "inotify_add_watch");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, mask);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int inotify_add_watch(int fd, const char *path, uint32_t mask)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s', mask: 0x%0x)\n", __func__, fd, path,
		mask);

	return next_inotify_add_watch(fd, path, mask);
}
#endif
