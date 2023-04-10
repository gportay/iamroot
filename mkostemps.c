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

__attribute__((visibility("hidden")))
int next_mkostemps(char *path, int suffixlen, int oflags)
{
	int (*sym)(char *, int, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "mkostemps");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(path, suffixlen, oflags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int mkostemps(char *path, int suffixlen, int oflags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	size_t len;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	ret = next_mkostemps(buf, suffixlen, oflags);
	if (ret == -1)
		goto exit;

	len = __strlen(path);
	memcpy(path, buf+__strlen(buf)-len, len);

exit:
	__debug("%s(path: '%s' -> '%s', oflags: 0%o)\n", __func__, path, buf,
		oflags);

	return ret;
}

#ifdef __GLIBC__
weak_alias(mkostemps, mkostemps64);
#endif
