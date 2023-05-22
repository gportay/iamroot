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

static int (*sym)(int, int, const char *);

__attribute__((visibility("hidden")))
int next_extattr_delete_fd(int fd, int attrnamespace, const char *attrname)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "extattr_delete_fd");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(fd, attrnamespace, attrname);
}

int extattr_delete_fd(int fd, int attrnamespace, const char *attrname)
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	const int oldattrnamespace = attrnamespace;
	const char *oldattrname = attrname;
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	(void)oldattrnamespace;
	(void)oldattrname;

	siz = fpath(fd, buf, sizeof(buf));
	if (siz == -1)
		goto exit;

	if (attrnamespace == EXTATTR_NAMESPACE_SYSTEM) {
		int ret;

		ret = _snprintf(extbuf, sizeof(extbuf), "%s%s",
				IAMROOT_EXTATTR_PREFIX, attrname);
		if (ret == -1)
			return -1;

		attrnamespace = EXTATTR_NAMESPACE_USER;
		attrname = extbuf;
	}

	ret = next_extattr_delete_fd(fd, attrnamespace, attrname);

exit:
	__debug("%s(fd: %i <-> '%s', attrnamespace: %i -> %i, attrname: '%s' -> '%s') -> %i\n",
		__func__, fd, buf, oldattrnamespace, attrnamespace,
		oldattrname, attrname, ret);

	return ret;
}
#endif
