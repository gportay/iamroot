/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

static int (*sym)(const char *);

hidden
int next_chdir(const char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "chdir");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path);
}

int chdir(const char *path)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_chdir(buf);
	if (ret == -1)
		goto exit;

	ret = __chrootdir(NULL);

exit:
	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
