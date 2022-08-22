/*
 * Copyright 2022 Gaël PORTAY
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
int next_mkostempsat(int fd, char *path, int suffixlen, int flags)
{
	int (*sym)(int, char *, int, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "mkostempsat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, suffixlen, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int mkostempsat(int fd, char *path, int suffixlen, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	size_t len;
	int ret;

	siz = path_resolution(fd, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	ret = next_mkostempsat(fd, buf, suffixlen, flags);
	if (ret == -1)
		goto exit;

	len = __strlen(path);
	memcpy(path, buf+__strlen(buf)-len, len);

exit:
	__debug("%s(fd: %d, path: '%s' -> '%s', flags: 0%o)\n", __func__, fd,
		path, buf, flags);

	return ret;
}
#endif
