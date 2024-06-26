/*
 * Copyright 2022-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined __FreeBSD__ || defined __NetBSD__
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>
#if defined __NetBSD__
#include <sys/param.h>
#endif

#include <sys/types.h>
#include <sys/extattr.h>

#include "iamroot.h"

#ifndef EXTATTR_MAXNAMELEN
#define EXTATTR_MAXNAMELEN 255
#endif

static int (*sym)(const char *, int, const char *);

hidden int next_extattr_delete_link(const char *path, int attrnamespace,
				    const char *attrname)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "extattr_delete_link");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, attrnamespace, attrname);
}

int extattr_delete_link(const char *path, int attrnamespace,
			const char *attrname)
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	const int oldattrnamespace = attrnamespace;
	const char *oldattrname = attrname;
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;
	(void)oldattrnamespace;
	(void)oldattrname;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	if (attrnamespace == EXTATTR_NAMESPACE_SYSTEM) {
		int n;

		n = _snprintf(extbuf, sizeof(extbuf), "%s%s",
			      IAMROOT_EXTATTR_PREFIX, attrname);
		if (n == -1)
			goto exit;

		attrnamespace = EXTATTR_NAMESPACE_USER;
		attrname = extbuf;
	}

	ret = next_extattr_delete_link(buf, attrnamespace, attrname);

exit:
	__debug("%s(path: '%s' -> '%s', attrnamespace: %i -> %i, attrname: '%s' -> '%s') -> %i\n",
		__func__, path, buf, oldattrnamespace, attrnamespace,
		oldattrname, attrname, ret);

	return ret;
}
#endif
