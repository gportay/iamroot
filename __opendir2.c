/*
 * Copyright 2022-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <dirent.h>

#include "iamroot.h"

static DIR *(*sym)(const char *, int);

hidden
DIR *next___opendir2(const char *path, int oflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "__opendir2");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(path, oflags);
}

DIR *__opendir2(const char *path, int oflags)
{
	char buf[PATH_MAX];
	DIR *ret = NULL;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next___opendir2(buf, oflags);

exit:
	__debug("%s(path: '%s' -> '%s', oflags: 0%o) -> %p\n", __func__, path,
		buf, oflags, ret);

	return ret;
}
#endif
