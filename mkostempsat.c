/*
 * Copyright 2022-2023 Gaël PORTAY
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

__attribute__((visibility("hidden")))
int next_mkostempsat(int dfd, char *path, int suffixlen, int oflags)
{
	int (*sym)(int, char *, int, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "mkostempsat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(dfd, path, suffixlen, oflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int mkostempsat(int dfd, char *path, int suffixlen, int oflags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	size_t len;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	ret = next_mkostempsat(dfd, buf, suffixlen, oflags);
	if (ret == -1)
		goto exit;

	len = __strlen(path);
	memcpy(path, buf+__strlen(buf)-len, len);

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', oflags: 0%o)\n",
		__func__, dfd, __fpath(dfd), path, buf, oflags);

	return ret;
}
#endif
