/*
 * Copyright 2021-2024 Gaël PORTAY
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

hidden ssize_t next_readlinkat(int dfd, const char *path, char *buf,
			       size_t bufsiz)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "readlinkat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(dfd, path, buf, bufsiz);
}

ssize_t readlinkat(int dfd, const char *path, char *buf, size_t bufsiz)
{
	ssize_t siz, ret = -1;
	char tmp2[PATH_MAX+1]; /* NULL-terminated */
	char tmp[PATH_MAX];
	const char *root;
	size_t len;

	/*
	 * According to symlink(7):
	 *
	 * Various system calls do not follow links in the basename component
	 * of a pathname, and operate on the symbolic link itself. They are:
	 * lchown(2), lgetxattr(2), llistxattr(2), lremovexattr(2),
	 * lsetxattr(2), lstat(2), readlink(2), rename(2), rmdir(2), and
	 * unlink(2).
	 */
	siz = path_resolution2(dfd, path, tmp, sizeof(tmp),
			       AT_SYMLINK_NOFOLLOW,
			       PATH_RESOLUTION_NOWALKALONG);
	if (siz == -1)
		goto exit;

#if defined __linux__ || defined __FreeBSD__ || defined __NetBSD__
	if (streq(tmp, "/proc/self/exe")) {
		const char *exe;

		exe = __getexe();
		if (exe) {
			ret = __strlen(exe);
			if ((size_t)ret > bufsiz)
				ret = bufsiz;
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
		if ((size_t)ret > bufsiz)
			ret = bufsiz;

		memcpy(buf, tmp2, ret);
	}

	return ret;
}

ssize_t __readlinkat_chk(int fd, const char *path, char *buf, size_t pathlen,
			 size_t bufsiz)
{
	(void)pathlen;

	__debug("%s(path: '%s', buf: %p, pathlen: %zu, bufsiz: %zu)\n",
		__func__, path, buf, pathlen, bufsiz);

	/* Forward to another function */
	return readlinkat(fd, path, buf, bufsiz);
}
