/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

static int (*sym)(const char *);

__attribute__((visibility("hidden")))
int next_remove(const char *path)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "remove");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path);
}

int remove(const char *path)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	ret = next_remove(buf);

exit:
	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
