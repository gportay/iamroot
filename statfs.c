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

#ifdef __linux__
#include <sys/statfs.h>
#endif

#if defined __FreeBSD__ || defined __OpenBSD__
#include <sys/param.h>
#include <sys/mount.h>
#endif

#include "iamroot.h"

static int (*sym)(const char *, struct statfs *);

__attribute__((visibility("hidden")))
int next_statfs(const char *path, struct statfs *statfsbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "statfs");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, statfsbuf);
}

int statfs(const char *path, struct statfs *statfsbuf)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_statfs(buf, statfsbuf);

exit:
	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}

#ifdef __GLIBC__
weak_alias(statfs, __statfs);
#endif
