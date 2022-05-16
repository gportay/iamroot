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
extern int next_stat(const char *, struct stat *);
extern int next_fstat(int, struct stat *);
extern int next_lstat(const char *, struct stat *);
extern int next_fstatat(int, const char *, struct stat *, int);
extern int next___fstat(int, struct stat *);
extern int next___xstat(int, const char *, struct stat *);
extern int next___fxstat(int, int, struct stat *);
extern int next___lxstat(int, const char *, struct stat *);
extern int next___fxstatat(int, int, const char *, struct stat *, int);
#ifdef __GLIBC__
extern int next_stat64(const char *, struct stat64 *);
extern int next_fstat64(int, struct stat64 *);
extern int next_lstat64(const char *, struct stat64 *);
extern int next_fstatat64(int, const char *, struct stat64 *, int);
extern int next___xstat64(int, const char *, struct stat64 *);
extern int next___fxstat64(int, int, struct stat64 *);
extern int next___lxstat64(int, const char *, struct stat64 *);
extern int next___fxstatat64(int, int, const char *, struct stat64 *, int);
extern int next_statx(int, const char *, int, unsigned int, struct statx *);
#endif
extern uid_t next_geteuid();

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
int fissymlinkat(int fd, const char *path, int flags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(fd, path, &statbuf, flags);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fissymlink(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int issymlink(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_lstat(path, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISLNK(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fisdirectoryat(int fd, const char *path, int flags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(fd, path, &statbuf, flags);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int isdirectory(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_lstat(path, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fisdirectory(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISDIR(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fisfileat(int fd, const char *path, int flags)
{
	struct stat statbuf;
	int ret;

	ret = next_fstatat(fd, path, &statbuf, flags);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int fisfile(int fd)
{
	struct stat statbuf;
	int ret;

	ret = next_fstat(fd, &statbuf);
	if (ret == -1)
		return ret;

	return S_ISREG(statbuf.st_mode);
}

__attribute__((visibility("hidden")))
int isfile(const char *path)
{
	struct stat statbuf;
	int ret;

	ret = next_lstat(path, &statbuf);
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
	return strcmp(getrootdir(), "/") != 0;
}

__attribute__((visibility("hidden")))
int rootstat(const char *path, struct stat *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_stat(path, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int rootfstat(int fd, struct stat *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_fstat(fd, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int rootlstat(const char *path, struct stat *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_lstat(path, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int rootfstatat(int fd, const char *path, struct stat *buf, int flags)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_fstatat(fd, path, buf, flags);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int __rootxstat(int ver, const char *path, struct stat *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___xstat(ver, path, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int __rootfxstat(int ver, int fd, struct stat *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___fxstat(ver, fd, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int __rootlxstat(int ver, const char *path, struct stat *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___lxstat(ver, path, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int __rootfxstatat(int ver, int fd, const char *path, struct stat *buf,
		   int flags)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___fxstatat(ver, fd, path, buf, flags);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int rootstat64(const char *path, struct stat64 *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_stat64(path, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int rootfstat64(int fd, struct stat64 *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_fstat64(fd, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int rootlstat64(const char *path, struct stat64 *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_lstat64(path, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int rootfstatat64(int fd, const char *path, struct stat64 *buf, int flags)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_fstatat64(fd, path, buf, flags);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int __rootxstat64(int ver, const char *path, struct stat64 *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___xstat64(ver, path, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int __rootfxstat64(int ver, int fd, struct stat64 *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___fxstat64(ver, fd, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int __rootlxstat64(int ver, const char *path, struct stat64 *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___lxstat64(ver, path, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int __rootfxstatat64(int ver, int fd, const char *path, struct stat64 *buf,
		     int flags)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___fxstatat64(ver, fd, path, buf, flags);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->st_uid == uid)
		buf->st_uid = geteuid();

	gid = getegid();
	if (buf->st_gid == gid)
		buf->st_gid = 0;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
int rootstatx(int fd, const char *path, int flags, unsigned int mask,
	      struct statx *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next_statx(fd, path, flags, mask, buf);
	if (ret == -1)
		goto exit;

	uid = next_geteuid();
	if (buf->stx_uid == uid)
		buf->stx_uid = geteuid();

	gid = getegid();
	if (buf->stx_gid == gid)
		buf->stx_gid = 0;

exit:
	return ret;
}
#endif

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
		if (strcmp(root, "/") != 0) {
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

		cwd = strncat(cwd, path, sizeof(buf) - len - 1);
		buf[sizeof(buf)-1] = 0;
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
