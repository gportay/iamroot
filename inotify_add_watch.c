/*
 * Copyright 2022-2024 Gaël PORTAY
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

static int (*sym)(int, const char *, uint32_t);

hidden
int next_inotify_add_watch(int fd, const char *path, uint32_t mask)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "inotify_add_watch");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd, path, mask);
}

int inotify_add_watch(int fd, const char *path, uint32_t mask)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_inotify_add_watch(fd, path, mask);

exit:
	__debug("%s(fd: %i, path: '%s', mask: 0x%0x) -> %i\n", __func__, fd,
		path, mask, ret);

	return ret;
}
#endif
