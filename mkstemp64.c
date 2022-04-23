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

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next_mkstemp64(char *path)
{
	int (*sym)(char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "mkstemp64");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int mkstemp64(char *path)
{
	char buf[PATH_MAX];
	char *real_path;
	size_t len;
	int ret;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, __func__);
		return -1;
	}

	ret = next_mkstemp64(real_path);
	if (ret == -1)
		goto exit;

	len = __strlen(path);
	memcpy(path, real_path+__strlen(real_path)-len, len);

exit:
	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return ret;
}
#endif
