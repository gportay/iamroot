/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "iamroot.h"

static int (*sym)(char *, int, int);

__attribute__((visibility("hidden")))
int next_mkostemps(char *path, int suffixlen, int oflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "mkostemps");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, suffixlen, oflags);
}

int mkostemps(char *path, int suffixlen, int oflags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	size_t len;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_mkostemps(buf, suffixlen, oflags);
	if (ret == -1)
		goto exit;

	len = __strlen(path);
	memcpy(path, buf+__strlen(buf)-len, len);

exit:
	__debug("%s(path: '%s' -> '%s', ..., oflags: 0%o) -> %i\n", __func__,
		path, buf, oflags, ret);

	return ret;
}

#ifdef __GLIBC__
weak_alias(mkostemps, mkostemps64);
#endif
