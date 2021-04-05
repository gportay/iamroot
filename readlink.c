/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);
#define __strlcmp(s1, s2) strncmp(s1, s2, strlen(s2))

extern const char *getrootdir();

__attribute__((visibility("hidden")))
ssize_t next_readlink(const char *path, char *buf, size_t bufsize)
{
	ssize_t (*sym)(const char *, char *, size_t);

	sym = dlsym(RTLD_NEXT, "readlink");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, buf, bufsize);
}

ssize_t readlink(const char *path, char *buf, size_t bufsize)
{
	char tmp[PATH_MAX];
	const char *root;
	char *real_path;
	ssize_t ret;
	size_t len;

	root = getrootdir();
	len = strlen(root);

	if (strcmp(path, "/proc/self/root") == 0) {
		ret = len;
		if ((size_t)ret > bufsize)
			ret = bufsize;
		memcpy(buf, root, ret);
		goto exit;
	}

	real_path = path_resolution(path, tmp, sizeof(tmp),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = next_readlink(real_path, buf, bufsize);
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
	__verbose("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return ret;
}
