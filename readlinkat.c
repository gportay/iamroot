/*
 * Copyright 2021-2023 Gaël PORTAY
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
ssize_t next_readlinkat(int dfd, const char *path, char *buf, size_t bufsize)
{
	ssize_t (*sym)(int, const char *, char *, size_t);
	ssize_t ret;

	sym = dlsym(RTLD_NEXT, "readlinkat");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(dfd, path, buf, bufsize);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

ssize_t readlinkat(int dfd, const char *path, char *buf, size_t bufsize)
{
	char tmp2[PATH_MAX+1]; /* NULL-terminated */
	char tmp[PATH_MAX];
	const char *root;
	ssize_t ret, siz;
	size_t len;

	root = __getrootdir();
	len = __strlen(root);

	if (__streq(path, "/proc/self/root")) {
		__notice("%s: ignoring path resolution '%s'\n", __func__,
			 path);
		ret = len;
		if ((size_t)ret > bufsize)
			ret = bufsize;
		memcpy(tmp2, root, ret);
		return ret;
	}

	siz = path_resolution(dfd, path, tmp, sizeof(tmp),
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
			memcpy(tmp2, exe, ret);

			__notice("%s: resolving to '%s'\n", path, exe);

			goto exit;
		}
	}

	ret = next_readlinkat(dfd, tmp, tmp2, sizeof(tmp2)-1); /* NULL-terminated */
	if (ret == -1)
		goto exit;
	tmp2[ret] = 0; /* ensure NULL-terminated */

	if (strncmp(root, "/", ret) == 0)
		goto exit;

	if (__strlcmp(tmp2, __getrootdir()) != 0)
		goto exit;

	memmove(tmp2, tmp2+len, __strlen(tmp2)-len+1); /* NULL-terminated */
	ret -= len;

	if (ret == 0)
		tmp2[ret++] = '/';

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ...)\n", __func__,
		dfd, __fpath(dfd), path, tmp);

	if (buf && ret >= 0) {
		if ((size_t)ret > bufsize)
			ret = bufsize;

		memcpy(buf, tmp2, ret);
	}

	return ret;
}
