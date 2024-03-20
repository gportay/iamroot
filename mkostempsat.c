/*
 * Copyright 2022-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "iamroot.h"

static int (*sym)(int, char *, int, int);

hidden
int next_mkostempsat(int dfd, char *path, int suffixlen, int oflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "mkostempsat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, suffixlen, oflags);
}

int mkostempsat(int dfd, char *path, int suffixlen, int oflags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	size_t len;

	siz = path_resolution(dfd, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_mkostempsat(dfd, buf, suffixlen, oflags);
	if (ret == -1)
		goto exit;

	len = __strlen(__basename(path));
	strncpy(&path[__strlen(path)-len], &buf[__strlen(buf)-len], len);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ..., oflags: 0%o) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, oflags, ret);

	return ret;
}
#endif
