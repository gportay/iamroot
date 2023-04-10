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
int next_extattr_delete_fd(int fd, int attrnamespace, const char *attrname)
{
	int (*sym)(int, int, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "extattr_delete_fd");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(fd, attrnamespace, attrname);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int extattr_delete_fd(int fd, int attrnamespace, const char *attrname)
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	const int oldattrnamespace = attrnamespace;
	const char *oldattrname = attrname;
	char buf[PATH_MAX];
	ssize_t siz;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, __func__);
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

	__debug("%s(fd: %i <-> '%s', attrnamespace: %i -> %i, attrname: '%s' -> '%s')\n",
		__func__, fd, buf, oldattrnamespace, attrnamespace,
		oldattrname, attrname);

	return next_extattr_delete_fd(fd, attrnamespace, attrname);
}
#endif
