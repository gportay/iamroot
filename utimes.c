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

#include <sys/time.h>

#include "iamroot.h"

static int (*sym)(const char *, const struct timeval[2]);

__attribute__((visibility("hidden")))
int next_utimes(const char *path, const struct timeval times[2])
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "utimes");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, times);
}

int utimes(const char *path, const struct timeval times[2])
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_utimes(buf, times);

	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}
