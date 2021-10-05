/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

#include "iamroot.h"

extern char *next_getcwd(char *, size_t);
extern int next_stat(const char *, struct stat *);
extern int next_fstat(int, const struct stat *);
extern int next_lstat(const char *, struct stat *);
extern int next_fstatat(int, const char *, struct stat *, int);
extern int next___fstat(int, struct stat *);
extern int next___xstat(int, const char *, struct stat *);
extern int next___fxstat(int, int, struct stat *);
extern int next___lxstat(int, const char *, struct stat *);
extern int next___fxstatat(int, int, const char *, struct stat *, int);
#ifdef __GLIBC__
extern int next_stat64(const char *, struct stat64 *);
extern int next_fstat64(int, const struct stat64 *);
extern int next_lstat64(const char *, struct stat64 *);
extern int next_fstatat64(int, const char *, struct stat64 *, int);
extern int next___fstat64(int, struct stat64 *);
extern int next___xstat64(int, const char *, struct stat64 *);
extern int next___fxstat64(int, int, struct stat64 *);
extern int next___lxstat64(int, const char *, struct stat64 *);
extern int next___fxstatat64(int, int, const char *, struct stat64 *, int);
extern int next_statx(int, const char *, int, unsigned int, struct statx *);
#endif
extern uid_t next_geteuid();

static int pathsetenv(const char *root, const char *name, const char *value,
		      int overwrite)
{
	size_t rootlen, vallen, newlen;

	newlen = 0;
	rootlen = strlen(root);

	vallen = strlen(value);
	if (vallen > 0) {
		char *token, *saveptr, val[vallen];

		newlen = vallen;
		newlen += rootlen;
		newlen++; /* NUL */

		__strncpy(val, value);
		token = strtok_r(val, ":", &saveptr);
		if (token && *token)
			while (strtok_r(NULL, ":", &saveptr))
				newlen += rootlen;
	}

	if (newlen > 0) {
		char *str, *token, *saveptr, val[vallen], new_value[newlen];

		str = new_value;
		__strncpy(val, value);
		token = strtok_r(val, ":", &saveptr);
		if (token && *token) {
			int n;

			n = snprintf(str, newlen, "%s%s", root, token);
			if ((n == -1) || (newlen < (size_t)n)) {
				errno = EOVERFLOW;
				return -1;
			}
			str += n;
			newlen -= n;
			while ((token = strtok_r(NULL, ":", &saveptr))) {
				n = snprintf(str, newlen, ":%s%s", root, token);
				if ((n == -1) || (newlen < (size_t)n)) {
					errno = EOVERFLOW;
					return -1;
				}
				str += n;
				newlen -= n;
			}
		}

		return setenv(name, new_value, overwrite);
	}

	return setenv(name, value, overwrite);
}

static inline char *rootdir()
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

	root = rootdir();
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
		__verbose("Exiting chroot: '%s'\n", root);
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
int __rootfstat(int fd, struct stat *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___fstat(fd, buf);
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
int __rootfstat64(int fd, struct stat64 *buf)
{
	uid_t uid;
	gid_t gid;
	int ret;

	ret = next___fstat64(fd, buf);
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
	char *real_path;
	int ret;

	/* Prepend chroot and current working directory for relative paths */
	if (path[0] != '/') {
		size_t len, rootlen;
		const char *root;
		char *cwd;

		cwd = buf;
		root = getrootdir();
		rootlen = strlen(root);
		if (strcmp(root, "/") != 0) {
			__strncpy(buf, root);
			cwd += rootlen;
		}

		cwd = getcwd(cwd, sizeof(buf)-rootlen);
		len = strlen(cwd);
		if (rootlen + len + 1 + strlen(path) + 1 > sizeof(buf)) {
			errno = ENAMETOOLONG;
			return -1;
		}

		cwd[len++] = '/';
		cwd[len] = 0;

		strncat(cwd, path, sizeof(buf) - len);
	} else {
		real_path = path_resolution(path, buf, sizeof(buf),
					    AT_SYMLINK_NOFOLLOW);
		if (!real_path) {
			perror("path_resolution");
			return -1;
		}
	}

	real_path = sanitize(buf, sizeof(buf));

	ret = setenv("PATH", getenv("IAMROOT_PATH") ?: "/bin:/usr/bin", 1);
	if (ret)
		goto exit;

	ret = pathsetenv(real_path, "LD_LIBRARY_PATH",
			 getenv("IAMROOT_LD_LIBRARY_PATH") ?: "/usr/lib:/lib",
			 1);
	if (ret)
		goto exit;

	ret = setrootdir(real_path);
	if (ret == -1)
		goto exit;

	__verbose("Enterring chroot: '%s'\n", real_path);

exit:
	__verbose("%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);
	__verbose("IAMROOT_PATH=%s\n", getenv("IAMROOT_ROOT"));
	__verbose("PATH=%s\n", getenv("PATH"));
	__verbose("LD_LIBRARY_PATH=%s\n", getenv("LD_LIBRARY_PATH"));

	return 0;
}
