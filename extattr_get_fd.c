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

#include <sys/types.h>
#include <sys/extattr.h>

#include "iamroot.h"

#ifndef EXTATTR_MAXNAMELEN
#define EXTATTR_MAXNAMELEN 255
#endif

static ssize_t (*sym)(int, int, const char *, void *, size_t);

hidden ssize_t next_extattr_get_fd(int fd, int attrnamespace,
				   const char *attrname, void *data,
				   size_t nbytes)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "extattr_get_fd");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd, attrnamespace, attrname, data, nbytes);
}

ssize_t extattr_get_fd(int fd, int attrnamespace, const char *attrname,
		       void *data, size_t nbytes)
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	const int oldattrnamespace = attrnamespace;
	const char *oldattrname = attrname;
	ssize_t siz, ret = -1;
	char buf[PATH_MAX];
	(void)oldattrnamespace;
	(void)oldattrname;

	siz = fpath(fd, buf, sizeof(buf));
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

	ret = next_extattr_get_fd(fd, attrnamespace, attrname, data, nbytes);

exit:
	__debug("%s(fd: %i <-> '%s', attrnamespace: %i -> %i, attrname: '%s' -> '%s', ...) -> %zi\n",
		__func__, fd, buf, oldattrnamespace, attrnamespace,
		oldattrname, attrname, ret);

	return ret;
}
#endif
