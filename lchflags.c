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

#include <sys/stat.h>
#include <unistd.h>

#include "iamroot.h"

static int (*sym)(const char *, unsigned long);

hidden int next_lchflags(const char *path, unsigned long flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "lchflags");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, flags);
}

int lchflags(const char *path, unsigned long flags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	ret = next_lchflags(path, flags);

exit:
	__debug("%s(path: '%s', ...) -> %i\n", __func__, path, ret);

	return ret;
}
#endif
