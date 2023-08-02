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

static ssize_t (*sym)(const char *, int, void *, size_t);

__attribute__((visibility("hidden")))
ssize_t next_extattr_list_link(const char *path, int attrnamespace, void *data,
			       size_t nbytes)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "extattr_list_link");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, attrnamespace, data, nbytes);
}

ssize_t extattr_list_link(const char *path, int attrnamespace, void *data,
			  size_t nbytes)
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	int oldattrnamespace = attrnamespace;
	ssize_t i, extsize, siz, ret = -1;
	unsigned char *pdata = data;
	char buf[PATH_MAX];
	(void)nbytes;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		goto exit;

	if (attrnamespace == EXTATTR_NAMESPACE_SYSTEM)
		attrnamespace = EXTATTR_NAMESPACE_USER;

	ret = next_extattr_list_link(path, attrnamespace, extbuf,
				     sizeof(extbuf));
	if (ret == -1)
		goto exit;

	extsize = ret;
	ret = 0;
	i = 0;
	while (i < extsize) {
		size_t len, off = 0;

		len = extbuf[i];
		if (!len)
			break;

		if ((oldattrnamespace == EXTATTR_NAMESPACE_SYSTEM) &&
		    __strneq(&extbuf[i+1], IAMROOT_EXTATTR_PREFIX)) /* len */
			off += sizeof(IAMROOT_EXTATTR_PREFIX)-1;

		if (pdata) {
			pdata[ret] = len - off;
			memcpy(data+ret+1, &extbuf[i+1+off], len-off); /* len */
		}

		i += len + 1; /* len */
		if (len != off)
			ret += len + 1 - off; /* len */
	}

exit:
	__debug("%s(path: '%s' -> '%s', attrnamespace: %i -> %i, ...) -> %zi\n",
		__func__, path, buf, oldattrnamespace, attrnamespace, ret);

	return ret;
}
#endif
