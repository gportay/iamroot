/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <dirent.h>

#include "iamroot.h"

static DIR *(*sym)(const char *);

__attribute__((visibility("hidden")))
DIR *next_opendir(const char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "opendir");

	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	return sym(path);
}

DIR *opendir(const char *path)
{
	char buf[PATH_MAX];
	DIR *ret = NULL;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_opendir(buf);

exit:
	__debug("%s(path: '%s' -> '%s') -> %p\n", __func__, path, buf, ret);

	return ret;
}

#ifdef __GLIBC__
weak_alias(opendir, __opendir);
#endif
