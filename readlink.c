/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
ssize_t next_readlink(const char *path, char *buf, size_t bufsize)
{
	ssize_t (*sym)(const char *, char *, size_t);
	ssize_t ret;

	sym = dlsym(RTLD_NEXT, "readlink");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, buf, bufsize);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
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
		__notice("%s: ignoring path resolution '%s'\n", __func__,
			 path);
		ret = len;
		if ((size_t)ret > bufsize)
			ret = bufsize;
		memcpy(buf, root, ret);
		return ret;
	}

	real_path = path_resolution(AT_FDCWD, path, tmp, sizeof(tmp),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		__pathperror(path, __func__);
		return -1;
	}

	if (strcmp(real_path, "/proc/self/exe") == 0) {
		const char *exe;
	       
		exe = __getexe();
		if (exe) {
			ret = strlen(exe);
			if ((size_t)ret > bufsize)
				ret = bufsize;
			memcpy(buf, exe, ret);

			__notice("%s: resolving to '%s'\n", path, exe);

			goto exit;
		}
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
	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return ret;
}
