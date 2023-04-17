/*
 * Copyright 2020-2023 Gaël PORTAY
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
extern int next_fstat(int, struct stat *);
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

	return __set_errno(ENOSPC, -1);
}

__attribute__((visibility("hidden")))
int __fissymlinkat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fissymlink(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __issymlink(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fisdirectoryat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __isdirectory(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, AT_SYMLINK_NOFOLLOW);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fisdirectory(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fisfileat(int dfd, const char *path, int atflags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(dfd, path, &statbuf, atflags);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __fisfile(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int __isfile(const char *path)
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

static inline int __setrootdir(const char *path)
{
	if (!path) {
		__info("Exiting chroot: '%s'\n", __getrootdir());
		return unsetenv("IAMROOT_ROOT");
	}

	__info("Enterring chroot: '%s'\n", path);
	return setenv("IAMROOT_ROOT", path, 1);
}

__attribute__((visibility("hidden")))
const char *__getrootdir()
{
	char *root;

	root = __getroot();
	if (!root)
		return "/";

	return root;
}

__attribute__((visibility("hidden")))
int __chrootdir(const char *cwd)
{
	char buf[PATH_MAX];
	const char *root;

	if (cwd == NULL)
		cwd = next_getcwd(buf, sizeof(buf));

	if (cwd == NULL)
		return -1;

	root = __getrootdir();
	if (__strlcmp(cwd, root) != 0)
		return __setrootdir(NULL);

	return 0;
}

__attribute__((visibility("hidden")))
int __inchroot()
{
	return !streq(__getrootdir(), "/");
}

__attribute__((visibility("hidden")))
char *__striprootdir(char *path)
{
	const char *root;
	size_t len, size;
	char *ret;

	if (!path || !*path)
		return __set_errno(EINVAL, NULL);

	root = __getrootdir();
	if (streq(root, "/"))
		return path;

	ret = path;
	size = __strlen(ret);
	len = __strlen(root);
	if (strneq(root, ret, len))
		memcpy(ret, &ret[len], __strlen(ret)-len+1); /* NULL-terminated */

	if (!*ret)
		_strncpy(ret, "/", size);

	return ret;
}

int chroot(const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	ret = setenv("PATH", getenv("IAMROOT_PATH") ?: _PATH_STDPATH, 1);
	if (ret == -1) {
		__envperror("PATH", "setenv");
		return -1;
	}

	ret = __setrootdir(buf);
	if (ret == -1) {
		__pathperror(buf, "__setrootdir");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return 0;
}
