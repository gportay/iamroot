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

#ifdef __NetBSD__
static int (*sym)(int, int, const char *, const void *, size_t);
#else
static ssize_t (*sym)(int, int, const char *, const void *, size_t);
#endif

#ifdef __NetBSD__
hidden int next_extattr_set_fd(int fd, int attrnamespace, const char *attrname,
			       const void *data, size_t nbytes)
#else
hidden ssize_t next_extattr_set_fd(int fd, int attrnamespace,
				   const char *attrname, const void *data,
				   size_t nbytes)
#endif
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "extattr_set_fd");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd, attrnamespace, attrname, data, nbytes);
}

#ifdef __NetBSD__
int extattr_set_fd(int fd, int attrnamespace, const char *attrname,
		   const void *data, size_t nbytes)
#else
ssize_t extattr_set_fd(int fd, int attrnamespace, const char *attrname,
		       const void *data, size_t nbytes)
#endif
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	const int oldattrnamespace = attrnamespace;
	const char *oldattrname = attrname;
	char buf[PATH_MAX];
#ifdef __NetBSD__
	int ret = -1;
#else
	ssize_t ret = -1;
#endif
	ssize_t siz;
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

	ret = next_extattr_set_fd(fd, attrnamespace, attrname, data, nbytes);

exit:
#ifdef __NetBSD__
	__debug("%s(fd: %i <-> '%s', attrnamespace: %i -> %i, attrname: '%s' -> '%s', ...) -> %i\n",
		__func__, fd, buf, oldattrnamespace, attrnamespace,
		oldattrname, attrname, ret);
#else
	__debug("%s(fd: %i <-> '%s', attrnamespace: %i -> %i, attrname: '%s' -> '%s', ...) -> %zi\n",
		__func__, fd, buf, oldattrnamespace, attrnamespace,
		oldattrname, attrname, ret);
#endif

	return ret;
}
#endif
