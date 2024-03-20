/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

static int (*sym)(int, const char *, int);

hidden
int next_unlinkat(int dfd, const char *path, int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "unlinkat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, atflags);
}

int unlinkat(int dfd, const char *path, int atflags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf),
			      atflags | AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	ret = next_unlinkat(dfd, buf, atflags);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, atflags, ret);

	return ret;
}
