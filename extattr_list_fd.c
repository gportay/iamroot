/*
 * Copyright 2022 Gaël PORTAY
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
ssize_t next_extattr_list_fd(int fd, int attrnamespace, void *data,
			     size_t nbytes)
{
	ssize_t (*sym)(int, int, void *, size_t);
	ssize_t ret;

	sym = dlsym(RTLD_NEXT, "extattr_list_fd");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, attrnamespace, data, nbytes);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

ssize_t extattr_list_fd(int fd, int attrnamespace, void *data, size_t nbytes)
{
	char extbuf[EXTATTR_MAXNAMELEN+1]; /* NULL-terminated */
	int oldattrnamespace = attrnamespace;
	unsigned char *pdata = data;
	ssize_t extsize, siz;
	char buf[PATH_MAX];
	ssize_t i, ret;
	(void)nbytes;

	siz = __procfdreadlink(fd, buf, sizeof(buf));
	if (siz == -1) {
		__fpathperror(fd, "__procfdreadlink");
		return -1;
	}
	buf[siz] = 0;

	if (attrnamespace == EXTATTR_NAMESPACE_SYSTEM)
		attrnamespace = EXTATTR_NAMESPACE_USER;

	__debug("%s(fd: %i -> '%s', attrnamespace: %i -> %i, ...)\n", __func__,
		fd, buf, oldattrnamespace, attrnamespace);

	extsize = next_extattr_list_fd(fd, attrnamespace, extbuf,
				       sizeof(extbuf));
	if (extsize == -1)
		return -1;

	ret = 0;
	i = 0;
	while (i < extsize) {
		size_t len, off = 0;

		len = extbuf[i];
		if (!len)
			break;

		if ((oldattrnamespace == EXTATTR_NAMESPACE_SYSTEM) &&
		    (__strncmp(&extbuf[i+1], IAMROOT_EXTATTR_PREFIX) == 0)) /* len */
			off += sizeof(IAMROOT_EXTATTR_PREFIX)-1;

		if (pdata) {
			pdata[ret] = len - off;
			memcpy(data+ret+1, &extbuf[i+1+off], len-off); /* len */
		}

		i += len + 1; /* len */
		if (len != off)
			ret += len + 1 - off; /* len */
	}

	return ret;
}
#endif
