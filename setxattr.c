/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <linux/limits.h>

#include <sys/types.h>
#include <sys/xattr.h>

#include "iamroot.h"

static int (*sym)(const char *, const char *, const void *, size_t, int);

__attribute__((visibility("hidden")))
int next_setxattr(const char *path, const char *name, const void *value,
		  size_t size, int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "setxattr");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, name, value, size, flags);
}

int setxattr(const char *path, const char *name, const void *value,
	     size_t size, int flags)
{
	char xbuf[XATTR_NAME_MAX+1]; /* NULL-terminated */
	ssize_t siz, ret = -1;
	char buf[PATH_MAX];

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	if (!__strneq(name, IAMROOT_XATTRS_PREFIX)) {
		int ret;

		ret = _snprintf(xbuf, sizeof(xbuf), "%s%s",
				IAMROOT_XATTRS_PREFIX, name);
		if (ret == -1)
			goto exit;

		name = xbuf;
	}

	ret = next_setxattr(buf, name, value, size, flags);

exit:
	__debug("%s(path: '%s' -> '%s', name: '%s' -> '%s', ..., flags: 0%x) -> %zi\n",
		__func__, path, buf, name, xbuf, flags, ret);

	return ret;
}
#endif
