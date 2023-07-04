/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined __FreeBSD__ || defined __OpenBSD__
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/param.h>
#include <sys/mount.h>

#include "iamroot.h"

static int (*sym)(const char *, fhandle_t *);

__attribute__((visibility("hidden")))
int next_getfh(const char *path, fhandle_t *fhp)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "getfh");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, fhp);
}

int getfh(const char *path, fhandle_t *fhp)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_getfh(path, fhp);

exit:
	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}
#endif
