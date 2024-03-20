/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <dlfcn.h>
#include <linux/limits.h>

#include <sys/types.h>
#include <sys/xattr.h>

#include "iamroot.h"

static int (*sym)(int, const char *, const void *, size_t, int);

hidden int next_fsetxattr(int fd, const char *name, const void *value,
			  size_t size, int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fsetxattr");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd, name, value, size, flags);
}

int fsetxattr(int fd, const char *name, const void *value, size_t size,
	      int flags)
{
	char xbuf[XATTR_NAME_MAX+1]; /* NULL-terminated */
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		goto exit;

	if (!__strneq(name, IAMROOT_XATTRS_PREFIX)) {
		int n;

		n = _snprintf(xbuf, sizeof(xbuf), "%s%s",
			      IAMROOT_XATTRS_PREFIX, name);
		if (n == -1)
			goto exit;

		name = xbuf;
	}

	ret = next_fsetxattr(fd, name, value, size, flags);

exit:
	__debug("%s(fd: %i <-> '%s', name: '%s', ..., flags: 0x%x) -> %i\n",
		__func__, fd, buf, name, flags, ret);

	return ret;
}
#endif
