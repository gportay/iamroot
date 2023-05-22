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

static ssize_t (*sym)(const char *, const char *, void *, size_t);

__attribute__((visibility("hidden")))
ssize_t next_lgetxattr(const char *path, const char *name, void *value,
		       size_t size)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "lgetxattr");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, name, value, size);
}

ssize_t lgetxattr(const char *path, const char *name, void *value, size_t size)
{
	char xbuf[XATTR_NAME_MAX+1]; /* NULL-terminated */
	ssize_t siz, ret = -1;
	char buf[PATH_MAX];

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	if (!__strneq(name, IAMROOT_XATTRS_PREFIX)) {
		int ret;

		ret = _snprintf(xbuf, sizeof(xbuf), "%s%s",
				IAMROOT_XATTRS_PREFIX, name);
		if (ret == -1)
			return -1;

		name = xbuf;
	}

	ret = next_lgetxattr(buf, name, value, size);

exit:
	__debug("%s(path: '%s' -> '%s', name: '%s' -> '%s', ...) -> %zi\n",
		__func__, path, buf, name, xbuf, ret);

	return ret;
}
#endif
