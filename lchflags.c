/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/stat.h>
#include <unistd.h>

#include "iamroot.h"

static int (*sym)(const char *, unsigned long);

__attribute__((visibility("hidden")))
int next_lchflags(const char *path, unsigned long flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "lchflags");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, flags);
}

int lchflags(const char *path, unsigned long flags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_lchflags(path, flags);

	__debug("%s(path: '%s', ...) -> %i\n", __func__, path, ret);

	return ret;
}
#endif
