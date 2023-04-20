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
char *__getenv(const char *name)
{
	char buf[BUFSIZ];
	int ret;

	ret = _snprintf(buf, sizeof(buf), "IAMROOT_%s", name);
	if (ret == -1)
		return NULL;

	return getenv(buf);
}

__attribute__((visibility("hidden")))
int __setenv(const char *name, const char *value, int overwrite)
{
	char buf[BUFSIZ];
	int ret;

	ret = _snprintf(buf, sizeof(buf), "IAMROOT_%s", name);
	if (ret == -1)
		return -1;

	return setenv(buf, value, overwrite);
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
	int ret;

	if (!path) {
		__info("Exiting chroot: '%s'\n", __getrootdir());
		ret = unsetenv("IAMROOT_ROOT");
		if (ret == -1)
			return __env_perror("IAMROOT_ROOT", "unsetenv", -1);

		return ret;
	}

	__info("Enterring chroot: '%s'\n", path);
	ret = setenv("IAMROOT_ROOT", path, 1);
	if (ret == -1)
		return __env_perror("IAMROOT_ROOT", "setenv", -1);

	return ret;
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
	if (!__strleq(cwd, root))
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
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = setenv("PATH", getenv("IAMROOT_PATH") ?: _PATH_STDPATH, 1);
	if (ret == -1)
		return __env_perror("PATH", "setenv", -1);

	ret = __setrootdir(buf);
	if (ret == -1)
		return -1;

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return 0;
}
