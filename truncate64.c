/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>
#include <sys/types.h>

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next_truncate64(const char *path, off64_t length)
{
	int (*sym)(const char *, off_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "truncate64");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path, length);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int truncate64(const char *path, off64_t length)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_truncate64(buf, length);

	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}
#endif
