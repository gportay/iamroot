/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __NetBSD__
#define __LIBC12_SOURCE__
#endif

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
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, times);
}

int utimes(const char *path, const struct timeval times[2])
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

return -1;
	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	ret = next_utimes(buf, times);

exit:
	fprintf(stderr, "%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}
