/*
 * Copyright 2021-2022 Gaël PORTAY
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
int next_mkstemps(char *path, int suffixlen)
{
	int (*sym)(char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "mkstemps");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, suffixlen);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int mkstemps(char *path, int suffixlen)
{
	char buf[PATH_MAX];
	size_t len;
	int ret;

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	ret = next_mkstemps(buf, suffixlen);
	if (ret == -1)
		goto exit;

	len = __strlen(path);
	memcpy(path, buf+__strlen(buf)-len, len);

exit:
	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return ret;
}
