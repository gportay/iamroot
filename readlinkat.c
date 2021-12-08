/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
ssize_t next_readlinkat(int fd, const char *path, char *buf, size_t bufsize)
{
	ssize_t (*sym)(int, const char *, char *, size_t);
	ssize_t ret;

	sym = dlsym(RTLD_NEXT, "readlinkat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, buf, bufsize);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

ssize_t readlinkat(int fd, const char *path, char *buf, size_t bufsize)
{
	char tmp[PATH_MAX];
	const char *root;
	char *real_path;
	ssize_t ret;
	size_t len;

	root = getrootdir();
	len = strlen(root);

	if (strcmp(path, "/proc/self/root") == 0) {
		__notice("%s: ignoring path resolution '%s'\n", __func__,
			 path);
		ret = len;
		if ((size_t)ret > bufsize)
			ret = bufsize;
		memcpy(buf, root, ret);
		return ret;
	}

	real_path = fpath_resolutionat(fd, path, tmp, sizeof(tmp), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	ret = next_readlinkat(fd, real_path, buf, bufsize);
	if (ret == -1)
		goto exit;

	if (strncmp(root, "/", ret) == 0)
		goto exit;

	if (__strlcmp(buf, getrootdir()) != 0)
		goto exit;

	memmove(buf, buf+len, strlen(buf)-len+1);
	ret -= len;

	if (ret == 0)
		buf[ret++] = '/';

exit:
	__verbose_func("%s(fd: %i, path: '%s' -> '%s', ...)\n", __func__, fd,
		       path, real_path);

	return ret;
}
