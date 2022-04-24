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
int next_mkostemp64(char *path, int flags)
{
	int (*sym)(char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "mkostemp64");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int mkostemp64(char *path, int flags)
{
	char buf[PATH_MAX];
	size_t len;
	int ret;

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	ret = next_mkostemp64(buf, flags);
	if (ret == -1)
		goto exit;

	len = __strlen(path);
	memcpy(path, buf+__strlen(buf)-len, len);

exit:
	__debug("%s(path: '%s' -> '%s', flags: 0%o)\n", __func__, path, buf,
		flags);

	return ret;
}
#endif
