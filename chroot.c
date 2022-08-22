/*
 * Copyright 2020-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <paths.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

#include "iamroot.h"

extern char *next_getcwd(char *, size_t);
extern int next_fstatat(int, const char *, struct stat *, int);

__attribute__((visibility("hidden")))
int _snprintf(char *buf, size_t bufsize, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vsnprintf(buf, bufsize, fmt, ap);
	va_end(ap);

	if (ret == -1)
		return -1;

	if ((size_t)ret < bufsize)
		return ret;

	errno = ENOSPC;
	return -1;
}

__attribute__((visibility("hidden")))
int fissymlinkat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fissymlink(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(fd, "", &statbuf, AT_EMPTY_PATH);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int issymlink(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fisdirectoryat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int isdirectory(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fisdirectory(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(fd, "", &statbuf, AT_EMPTY_PATH);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fisfileat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fisfile(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(fd, "", &statbuf, AT_EMPTY_PATH);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int isfile(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __getfatal()
{
	return strtol(getenv("IAMROOT_FATAL") ?: "0", NULL, 0);
}

__attribute__((visibility("hidden")))
char *__getroot()
{
	return getenv("IAMROOT_ROOT");
}

static inline int setrootdir(const char *path)
{
	if (!path)
		return unsetenv("IAMROOT_ROOT");

	return setenv("IAMROOT_ROOT", path, 1);
}

__attribute__((visibility("hidden")))
const char *getrootdir()
{
	char *root;

	root = __getroot();
	if (!root)
		return "/";

	return root;
}

__attribute__((visibility("hidden")))
int chrootdir(const char *cwd)
{
	char buf[PATH_MAX];
	const char *root;

	if (cwd == NULL)
		cwd = next_getcwd(buf, sizeof(buf));

	if (cwd == NULL)
		return -1;

	root = getrootdir();
	if (__strlcmp(cwd, root) != 0) {
		__info("Exiting chroot: '%s'\n", root);
		return unsetenv("IAMROOT_ROOT");
	}

	return 0;
}

__attribute__((visibility("hidden")))
int inchroot()
{
	return !__streq(getrootdir(), "/");
}

__attribute__((visibility("hidden")))
char *striprootdir(char *path)
{
	const char *root;
	size_t len, size;
	char *ret;

	if (!path || !*path) {
		errno = EINVAL;
		return NULL;
	}

	root = getrootdir();
	if (__streq(root, "/"))
		return path;

	ret = path;
	size = __strlen(ret);
	len = __strlen(root);
	if (strncmp(root, ret, len) == 0)
		memcpy(ret, &ret[len], __strlen(ret)-len+1); /* NULL-terminated */

	if (!*ret)
		_strncpy(ret, "/", size);

	return ret;
}

int chroot(const char *path)
{
	char buf[PATH_MAX];
	int ret;

	/* Prepend chroot and current working directory for relative paths */
	if (path[0] != '/') {
		size_t len, rootlen;
		const char *root;
		char *cwd;

		cwd = buf;
		root = getrootdir();
		rootlen = __strlen(root);
		if (!__streq(root, "/")) {
			__strncpy(buf, root);
			cwd += rootlen;
		}

		cwd = getcwd(cwd, sizeof(buf)-rootlen);
		len = __strlen(cwd);
		if (rootlen + len + 1 + __strlen(path) + 1 > sizeof(buf)) {
			errno = ENAMETOOLONG;
			return -1;
		}

		cwd[len++] = '/';
		cwd[len] = 0;

		cwd = _strncat(cwd, path, sizeof(buf)-len);
		buf[sizeof(buf)-1] = 0; /* NULL-terminated */
	} else {
		ssize_t siz;

		siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
				      AT_SYMLINK_NOFOLLOW);
		if (siz == -1) {
			__pathperror(path, __func__);
			return -1;
		}
	}

	sanitize(buf, sizeof(buf));

	ret = setenv("PATH", getenv("IAMROOT_PATH") ?: _PATH_STDPATH, 1);
	if (ret == -1) {
		__envperror("PATH", "setenv");
		return -1;
	}

	ret = setrootdir(buf);
	if (ret == -1) {
		__pathperror(buf, "setrootdir");
		return -1;
	}

	__info("Enterring chroot: '%s'\n", buf);
	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return 0;
}
