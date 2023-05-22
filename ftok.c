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

#include <sys/ipc.h>

#include "iamroot.h"

static int (*sym)(const char *, int);

__attribute__((visibility("hidden")))
key_t next_ftok(const char *path, int proj_id)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "ftok");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, proj_id);
}

key_t ftok(const char *path, int proj_id)
{
	char buf[PATH_MAX];
	ssize_t siz;
	key_t ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_ftok(buf, proj_id);

	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}
