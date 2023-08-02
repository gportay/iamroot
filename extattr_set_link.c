/*
 * Copyright 2022-2023 Gaël PORTAY
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
static int (*sym)(const char *, int, const char *, const void *, size_t);
#else
static ssize_t (*sym)(const char *, int, const char *, const void *, size_t);
#endif

#ifdef __NetBSD__
__attribute__((visibility("hidden")))
int next_extattr_set_link(const char *path, int attrnamespace,
			  const char *attrname, const void *data,
			  size_t nbytes)
#else
__attribute__((visibility("hidden")))
ssize_t next_extattr_set_link(const char *path, int attrnamespace,
			      const char *attrname, const void *data,
			      size_t nbytes)
#endif
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "extattr_set_link");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, attrnamespace, attrname, data, nbytes);
}

#ifdef __NetBSD__
int extattr_set_link(const char *path, int attrnamespace, const char *attrname,
		     const void *data, size_t nbytes)
#else
ssize_t extattr_set_link(const char *path, int attrnamespace,
			 const char *attrname, const void *data, size_t nbytes)
#endif
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	const int oldattrnamespace = attrnamespace;
	const char *oldattrname = attrname;
#ifdef __NetBSD__
	int ret = -1;
#else
	ssize_t ret = -1;
#endif
	ssize_t siz;
	char buf[PATH_MAX];
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

	ret = next_extattr_set_link(buf, attrnamespace, attrname, data,
				    nbytes);

exit:
	__debug("%s(path: '%s' -> '%s', attrnamespace: %i -> %i, attrname: '%s' -> '%s', ...)\n",
		__func__, path, buf, oldattrnamespace, attrnamespace,
		oldattrname, attrname);

	return ret;
}
#endif
