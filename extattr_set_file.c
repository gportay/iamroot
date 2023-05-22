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

static int (*sym)(const char *, int, const char *, const void *, size_t);

__attribute__((visibility("hidden")))
ssize_t next_extattr_set_file(const char *path, int attrnamespace,
			      const char *attrname, const void *data,
			      size_t nbytes)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "extattr_set_file");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, attrnamespace, attrname, data, nbytes);
}

ssize_t extattr_set_file(const char *path, int attrnamespace,
			 const char *attrname, const void *data, size_t nbytes)
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	const int oldattrnamespace = attrnamespace;
	const char *oldattrname = attrname;
	char buf[PATH_MAX];
	ssize_t ret, siz;
	(void)oldattrnamespace;
	(void)oldattrname;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	if (attrnamespace == EXTATTR_NAMESPACE_SYSTEM) {
		int ret;

		ret = _snprintf(extbuf, sizeof(extbuf), "%s%s",
				IAMROOT_EXTATTR_PREFIX, attrname);
		if (ret == -1)
			return -1;

		attrnamespace = EXTATTR_NAMESPACE_USER;
		attrname = extbuf;
	}

	ret = next_extattr_set_file(buf, attrnamespace, attrname, data,
				    nbytes);

	__debug("%s(path: '%s' -> '%s', attrnamespace: %i -> %i, attrname: '%s' -> '%s', ...) -> %zi\n",
		__func__, path, buf, oldattrnamespace, attrnamespace,
		oldattrname, attrname, ret);

	return ret;
}
#endif
