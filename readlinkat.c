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

static ssize_t (*sym)(int, const char *, char *, size_t);

__attribute__((visibility("hidden")))
ssize_t next_readlinkat(int dfd, const char *path, char *buf, size_t bufsize)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "readlinkat");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(dfd, path, buf, bufsize);
}

ssize_t readlinkat(int dfd, const char *path, char *buf, size_t bufsize)
{
	char tmp2[PATH_MAX+1]; /* NULL-terminated */
	char tmp[PATH_MAX];
	const char *root;
	ssize_t ret, siz;
	size_t len;

	siz = path_resolution(dfd, path, tmp, sizeof(tmp),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

#if defined __linux__ || defined __FreeBSD__
	if (streq(tmp, "/proc/self/exe")) {
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
#endif

	ret = next_readlinkat(dfd, tmp, tmp2, sizeof(tmp2)-1); /* NULL-terminated */
	if (ret == -1)
		goto exit;
	tmp2[ret] = 0; /* ensure NULL-terminated */

	root = __getrootdir();
	len = __strlen(root);

	if (strneq(root, "/", ret))
		goto exit;

	if (!__strleq(tmp2, __getrootdir()))
		goto exit;

	memmove(tmp2, tmp2+len, __strlen(tmp2)-len+1); /* NULL-terminated */
	ret -= len;

	if (ret == 0)
		tmp2[ret++] = '/';

exit:
	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ...) -> %zi\n",
		__func__, dfd, __fpath(dfd), path, tmp, ret);

	if (buf && ret >= 0) {
		if ((size_t)ret > bufsize)
			ret = bufsize;

		memcpy(buf, tmp2, ret);
	}

	return ret;
}
