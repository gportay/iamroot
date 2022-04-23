/*
 * Copyright 2020-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <paths.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <regex.h>

#include <fcntl.h>

#include "iamroot.h"

/* AT flag FOLLOW takes precedence over NOFOLLOW */
#define follow_symlink(f) ((f & AT_SYMLINK_FOLLOW) || (f & AT_SYMLINK_NOFOLLOW) == 0)

extern char *next_realpath(const char *, char *);
extern ssize_t next_readlink(const char *, char *, size_t);
extern int next_lstat(const char *, struct stat *);

/*
 * Slolen from musl (src/internal/procfdname.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
__attribute__((visibility("hidden")))
void __procfdname(char *buf, unsigned fd)
{
	unsigned i, j;
	for (i=0; (buf[i] = "/proc/self/fd/"[i]); i++);
	if (!fd) {
		buf[i] = '0';
		buf[i+1] = 0;
		return;
	}
	for (j=fd; j; j/=10, i++);
	buf[i] = 0;
	for (; fd; fd/=10) buf[--i] = '0' + fd%10;
}

__attribute__((visibility("hidden")))
ssize_t __procfdreadlink(int fd, char *buf, size_t bufsize)
{
	char tmp[sizeof("/proc/self/fd/") + 4];
	__procfdname(tmp, fd);
	return next_readlink(tmp, buf, bufsize);
}

static regex_t *re;

static void __regex_perror(const char *s, regex_t *regex, int err)
{
	char buf[128];
	regerror(err, regex, buf, sizeof(buf));
	if (!s) {
		dprintf(STDERR_FILENO, "%s\n", buf);
		return;
	}

	dprintf(STDERR_FILENO, "%s: %s\n", s, buf);
}

__attribute__((optimize("O0")))
__attribute__((constructor,visibility("hidden")))
void path_resolution_init()
{
	const char *ignore, *exec;
	static regex_t regex;
#ifndef JIMREGEXP_H
	__attribute__((unused)) static char jimpad[40];
#endif
	char buf[BUFSIZ];
	int ret;

	if (re)
		return;

	ignore = getenv("IAMROOT_PATH_RESOLUTION_IGNORE");
	if (!ignore)
		ignore = "^/proc/|/sys/|"_PATH_DEV"|"_PATH_VARRUN"|/run/";

	exec = getenv("IAMROOT_EXEC");
	if (!exec)
		exec = "^/usr/lib/iamroot/exec.sh$";

	ret = _snprintf(buf, sizeof(buf), "%s|%s", ignore, exec);
	if (ret == -1) {
		perror("_snprintf");
		return;
	}

	ret = regcomp(&regex, buf, REG_NOSUB|REG_EXTENDED);
	if (ret) {
		__regex_perror("regcomp", &regex, ret);
		return;
	}

	__info("IAMROOT_PATH_RESOLUTION_IGNORE|IAMROOT_EXEC=%s\n", buf);

	re = &regex;
}

__attribute__((destructor,visibility("hidden")))
void path_resolution_fini()
{
	if (!re)
		return;

	regfree(re);
	re = NULL;
}

static int ignore(const char *path)
{
	int ret = 0;

	if (!re)
		return 0;

	if (!*path)
		return 0;

	/*
	 * TODO Create a new environment variable to white list paths to be
	 * ignored?
	 */
	if (__strncmp(path, "/run/systemd") == 0)
		return 0;

	ret = regexec(re, path, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re, ret);
		return 0;
	}

	return !ret;
}

char *sanitize(char *path, size_t bufsize)
{
	ssize_t len;

	if (!*path)
		return path;

	len = strnlen(path, bufsize);
	while ((len > 3) && (__strncmp(path, "./") == 0) && path[2] != '/') {
		char *s;
		for (s = path; *s; s++)
			*s = *(s+2);
		len -= 2;
	}
	while ((len > 2) && (__strncmp(&path[len-2], "/.") == 0)) {
		path[len-2] = 0;
		len -= 2;
	}
	while ((len > 1) && (path[len-1] == '/')) {
		path[len-1] = 0;
		len--;
	}
	if (len == 0) {
		path[len++] = '.';
		path[len] = 0;
	}

	return path;
}

__attribute__((visibility("hidden")))
char *fpath(int fd, char *buf, size_t bufsiz)
{
	ssize_t siz;

	siz = __procfdreadlink(fd, buf, bufsiz);
	if (siz == -1)
		return NULL;
	buf[siz] = 0; /* ensure NULL terminated */

	return buf;
}

static char *__fpath(int fd)
{
	static char buf[PATH_MAX];

	*buf = 0;
	return fpath(fd, buf, sizeof(buf));
}

__attribute__((visibility("hidden")))
int path_ignored(int fd, const char *path)
{
	if (fd != AT_FDCWD) {
		char buf[PATH_MAX];
		char *real_path;

		real_path = fpath(fd, buf, sizeof(buf));
		if (!real_path)
			return -1;

		return ignore(real_path);
	}

	return ignore(path);
}

static char *_path_resolution(int fd, const char *path, char *buf,
			      size_t bufsize, int flags, int symlinks)
{
	const char *root;
	char *real_path;
	size_t len;
	int ret;

	if (fd == -1 || !path) {
		errno = EINVAL;
		return NULL;
	}

	if (ignore(path))
		goto ignore;

	if (*path == '/') {
		const char *root;
		int size;

		root = getrootdir();
		if (strcmp(root, "/") == 0)
			root = "";
		else if (__strlcmp(path, root) == 0)
			__warning("%s: contains root '%s'\n", path, root);

		size = _snprintf(buf, bufsize, "%s%s", root, path);
		if (size < 0) {
			errno = EINVAL;
			return NULL;
		}

		if ((size_t)size >= bufsize) {
			errno = ENAMETOOLONG;
			return NULL;
		}
	} else if (fd != AT_FDCWD) {
		char dirbuf[PATH_MAX];
		char *real_dir;
		int size;

		real_dir = fpath(fd, dirbuf, sizeof(dirbuf));
		if (!real_dir) {
			__fpathperror(fd, "fpath");
			return NULL;
		}

		if (*real_dir != '/') {
			__warning("%s: ignore '/proc/self/fd/%d'\n", real_dir,
				  fd);
			goto ignore;
		}

		size = _snprintf(buf, bufsize, "%s/%s", real_dir, path);
		if (size < 0) {
			errno = EINVAL;
			return NULL;
		}

		if ((size_t)size >= bufsize) {
			errno = ENAMETOOLONG;
			return NULL;
		}
	} else {
		_strncpy(buf, path, bufsize);
	}

	real_path = sanitize(buf, bufsize);

	root = getrootdir();
	if (strcmp(root, "/") == 0)
		root = "";
	len = __strlen(root);

	if (ignore(real_path+len))
		return memcpy(buf, real_path+len, __strlen(real_path+len)+1);

	if (!follow_symlink(flags))
		goto exit;

	symlinks++;
	if (symlinks >= MAXSYMLINKS) {
		errno = ELOOP;
		return NULL;
	}

	ret = issymlink(real_path);
	if (ret == -1)
		goto exit;

	if (ret) {
		char tmp[NAME_MAX];
		ssize_t s;

		s = next_readlink(real_path, tmp, sizeof(tmp) - 1);
		if (s == -1)
			return NULL;
		tmp[s] = 0; /* ensure NULL terminated */

		if (*tmp != '/') {
			char *basename, *real_tmp, tmpbuf[PATH_MAX];

			real_tmp = __strncpy(tmpbuf, path);
			sanitize(real_tmp, sizeof(tmpbuf));

			basename = __basename(real_tmp);
			strcpy(basename, tmp);
			sanitize(real_tmp, sizeof(tmpbuf));

			return _path_resolution(AT_FDCWD, real_tmp, buf,
						bufsize, flags, symlinks);
		}

		return _path_resolution(AT_FDCWD, tmp, buf, bufsize, flags,
					symlinks);
	}

exit:
	return real_path;

ignore:
	return _strncpy(buf, path, bufsize);
}

char *path_resolution(int fd, const char *path, char *buf, size_t bufsize,
		      int flags)
{
	return _path_resolution(fd, path, buf, bufsize, flags, 0);
}

__attribute__((visibility("hidden")))
char *__getpath(int fd, const char *path, int flags)
{
	static char buf[PATH_MAX];

	*buf = 0;
	if (path_ignored(fd, path) > 0) {
		if (!*path)
			path = __fpath(fd);

		if (follow_symlink(flags))
			return next_realpath(path, buf);

		return _strncpy(buf, path, sizeof(buf));
	}

	return path_resolution(fd, path, buf, sizeof(buf), flags);
}

/*
 * Stolen from musl (src/process/execvp.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
__attribute__((visibility("hidden")))
char *path_access(const char *file, int mode, const char *path, char *buf,
		  size_t bufsiz)
{
	const char *p, *z;
	size_t l, k;
	int seen_eacces = 0;

	errno = ENOENT;
	if (!*file) return NULL;

	if (!path) return NULL;
	k = strnlen(file, NAME_MAX+1);
	if (k > NAME_MAX) {
		errno = ENAMETOOLONG;
		return NULL;
	}
	l = strnlen(path, PATH_MAX-1)+1;

	for(p=path; ; p=z) {
		char b[l+k+1];
		z = __strchrnul(p, ':');
		if ((size_t)(z-p) >= l) {
			if (!*z++) break;
			continue;
		}
		memcpy(b, p, z-p);
		b[z-p] = '/';
		memcpy(b+(z-p)+(z>p), file, k+1);

		if (access(b, mode) != -1)
			return path_resolution(AT_FDCWD, b, buf, bufsiz, 0);
		switch (errno) {
		case EACCES:
			seen_eacces = 1;
		case ENOENT:
		case ENOTDIR:
			break;
		default:
			return NULL;
		}
		if (!*z++) break;
	}
	if (seen_eacces) errno = EACCES;
	return NULL;
}
