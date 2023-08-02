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
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, proj_id);
}

key_t ftok(const char *path, int proj_id)
{
	key_t ret = (key_t)-1;
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_ftok(buf, proj_id);

exit:
	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		(int)ret);

	return ret;
}
