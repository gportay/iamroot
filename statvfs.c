/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/statvfs.h>

#include "iamroot.h"

static int (*sym)(const char *, struct statvfs *);

__attribute__((visibility("hidden")))
int next_statvfs(const char *path, struct statvfs *statvfsbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "statvfs");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, statvfsbuf);
}

int statvfs(const char *path, struct statvfs *statvfsbuf)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_statvfs(buf, statvfsbuf);

exit:
	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}
