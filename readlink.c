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
	ssize_t ret, siz;
	size_t len;

	root = getrootdir();
	len = __strlen(root);

	if (__streq(path, "/proc/self/root")) {
		__notice("%s: ignoring path resolution '%s'\n", __func__,
			 path);
		ret = len;
		if ((size_t)ret > bufsize)
			ret = bufsize;
		memcpy(buf, root, ret);
		return ret;
	}

	siz = path_resolution(AT_FDCWD, path, tmp, sizeof(tmp),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	if (__streq(tmp, "/proc/self/exe")) {
		const char *exe;
	       
		exe = __getexe();
		if (exe) {
			ret = __strlen(exe);
			if ((size_t)ret > bufsize)
				ret = bufsize;
			memcpy(buf, exe, ret);

			__notice("%s: resolving to '%s'\n", path, exe);

			goto exit;
		}
	}

	ret = next_readlink(tmp, buf, bufsize);
	if (ret == -1)
		goto exit;

	if (strncmp(root, "/", ret) == 0)
		goto exit;

	if (__strlcmp(buf, getrootdir()) != 0)
		goto exit;

	memmove(buf, buf+len, strnlen(buf, bufsize)-len+1);
	ret -= len;

	if (ret == 0)
		buf[ret++] = '/';

exit:
	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, tmp);

	return ret;
}
