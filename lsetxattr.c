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
#include <fcntl.h>
#include <dlfcn.h>
#include <linux/limits.h>

#include <sys/types.h>
#include <sys/xattr.h>

#include "iamroot.h"

static int (*sym)(const char *, const char *, const void *, size_t, int);

hidden int next_lsetxattr(const char *path, const char *name,
			  const void *value, size_t size, int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "lsetxattr");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, name, value, size, flags);
}

int lsetxattr(const char *path, const char *name, const void *value,
	      size_t size, int flags)
{
	char xbuf[XATTR_LIST_MAX+1]; /* NULL-terminated */
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	/*
	 * According to symlink(7):
	 *
	 * Various system calls do not follow links in the basename component
	 * of a pathname, and operate on the symbolic link itself. They are:
	 * lchown(2), lgetxattr(2), llistxattr(2), lremovexattr(2),
	 * lsetxattr(2), lstat(2), readlink(2), rename(2), rmdir(2), and
	 * unlink(2).
	 */
	siz = path_resolution2(AT_FDCWD, path, buf, sizeof(buf),
			       AT_SYMLINK_NOFOLLOW,
			       PATH_RESOLUTION_NOWALKALONG);
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

	ret = next_lsetxattr(buf, name, value, size, flags);

exit:
	__debug("%s(path: '%s' -> '%s', name: '%s' -> '%s', ...) -> %i\n",
		__func__, path, buf, name, xbuf, ret);

	return ret;
}
#endif
