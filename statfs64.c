/*
 * Copyright 2021-2024 Gaël PORTAY
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

#ifdef __FreeBSD__
#include <sys/param.h>
#include <sys/mount.h>
#endif

#include "iamroot.h"

#ifdef __GLIBC__
static int (*sym)(const char *, struct statfs64 *);

hidden
int next_statfs64(const char *path, struct statfs64 *statfsbuf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "statfs64");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, statfsbuf);
}

int statfs64(const char *path, struct statfs64 *statfsbuf)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_statfs64(buf, statfsbuf);

exit:
	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}

int __statfs64 (const char *__file, struct statfs64 *__buf)
     __THROW __nonnull ((1, 2));
weak_alias(statfs64, __statfs64);
#endif
