/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __FreeBSD__
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/extattr.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
ssize_t next_extattr_set_link(const char *path, int attrnamespace,
			      const char *attrname, const void *data,
			      size_t nbytes)
{
	int (*sym)(const char *, int, const char *, const void *, size_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "extattr_set_link");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path, attrnamespace, attrname, data, nbytes);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

ssize_t extattr_set_link(const char *path, int attrnamespace,
			 const char *attrname, const void *data, size_t nbytes)
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	const int oldattrnamespace = attrnamespace;
	const char *oldattrname = attrname;
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	if (attrnamespace == EXTATTR_NAMESPACE_SYSTEM) {
		int ret;

		ret = _snprintf(extbuf, sizeof(extbuf), "%s%s",
				IAMROOT_EXTATTR_PREFIX, attrname);
		if (ret == -1)
			return -1;

		attrnamespace = EXTATTR_NAMESPACE_USER;
		attrname = extbuf;
	}

	__debug("%s(path: '%s' -> '%s', attrnamespace: %i -> %i, attrname: '%s' -> '%s', ...)\n",
		__func__, path, buf, oldattrnamespace, attrnamespace,
		oldattrname, attrname);

	return next_extattr_set_link(buf, attrnamespace, attrname, data,
				     nbytes);
}
#endif
