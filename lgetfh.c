/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
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
int next_lgetfh(const char *path, fhandle_t *fhp)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "lgetfh");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, fhp);
}

int lgetfh(const char *path, fhandle_t *fhp)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_lgetfh(path, fhp);

	__debug("%s(path: '%s' -> '%s', ...) -> %i\n", __func__, path, buf,
		ret);

	return ret;
}
#endif
