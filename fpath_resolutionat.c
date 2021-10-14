/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <regex.h>

#include <fcntl.h>

#include "iamroot.h"

extern ssize_t next_readlink(const char *, char *, size_t);
extern int next_lstat(const char *, struct stat *);

static inline ssize_t __procfdreadlink(int fd, char *buf, size_t bufsize)
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
		fprintf(stderr, "%s\n", buf);
		return;
	}

	fprintf(stderr, "%s: %s\n", s, buf);
}

__attribute__((constructor))
void path_resolution_init()
{
	const char *ignore, *library, *library_musl_x86_64, *exec;
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
		ignore = "^/(proc|sys|dev|run)/";

	library = getenv("IAMROOT_LIB");
	if (!library)
		library = "^/usr/lib/iamroot/libiamroot.so$";

	library_musl_x86_64 = getenv("IAMROOT_LIB_MUSL_X86_64");
	if (!library_musl_x86_64)
		library_musl_x86_64 = "^/usr/lib/iamroot/ld-musl-x86_64.so.1$";

	exec = getenv("IAMROOT_EXEC");
	if (!exec)
		exec = "^/usr/lib/iamroot/exec.sh$";

	snprintf(buf, sizeof(buf)-1, "%s|%s|%s|%s", ignore, library,
		 library_musl_x86_64, exec);

	ret = regcomp(&regex, buf, REG_NOSUB|REG_EXTENDED);
	if (ret) {
		__regex_perror("regcomp", &regex, ret);
		return;
	}

	__verbose("IAMROOT_PATH_RESOLUTION_IGNORE|IAMROOT_LIB|IAMROOT_LIB_MUSL_X86_64|IAMROOT_EXEC=%s\n",
		  buf);

	re = &regex;
}

__attribute__((destructor))
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

	len = strnlen(path, bufsize);
	while ((len > 3) && (__strncmp(path, "./") == 0)) {
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

char *fpath_resolutionat(int fd, const char *path, char *buf, size_t bufsize,
			 int flags)
{
	struct stat statbuf;
	const char *root;
	char *real_path;
	size_t len;

	if (fd == -1 || !path) {
		errno = EINVAL;
		return NULL;
	}

	if (ignore(path))
		return strncpy(buf, path, bufsize);

	if (*path == '/') {
		const char *root;
		int size;

		root = getrootdir();
		if (strcmp(root, "/") == 0)
			root = "";
		else if (__strlcmp(path, root) == 0)
			__warning("%s: contains root '%s'\n", path, root);

		size = snprintf(buf, bufsize, "%s%s", root, path);
		if (size < 0) {
			errno = EINVAL;
			return NULL;
		}

		if ((size_t)size >= bufsize) {
			errno = ENAMETOOLONG;
			return NULL;
		}
	} else if (fd != AT_FDCWD) {
		char dir[PATH_MAX];
		ssize_t siz;
		int size;

		siz = __procfdreadlink(fd, dir, sizeof(dir));
		if (siz == -1) {
			perror("__procfdreadlink");
			return NULL;
		}

		dir[siz] = 0;
		size = snprintf(buf, bufsize, "%s/%s", dir, path);
		if (size < 0) {
			errno = EINVAL;
			return NULL;
		}

		if ((size_t)size >= bufsize) {
			errno = ENAMETOOLONG;
			return NULL;
		}
	} else {
		strncpy(buf, path, bufsize);
	}

	real_path = sanitize(buf, bufsize);

	root = getrootdir();
	if (strcmp(root, "/") == 0)
		root = "";
	len = strlen(root);

	if (ignore(real_path+len))
		return memcpy(buf, real_path+len, strlen(real_path+len)+1);

	/* AT flag FOLLOW takes precedence over NOFOLLOW */
	if ((flags & AT_SYMLINK_FOLLOW) != 0)
		goto symlink_follow;

	if ((flags & AT_SYMLINK_NOFOLLOW) != 0)
		goto exit;

symlink_follow:
	if (next_lstat(real_path, &statbuf) != 0)
		goto exit;

	if (S_ISLNK(statbuf.st_mode)) {
		char tmp[NAME_MAX];
		ssize_t s;

		s = next_readlink(real_path, tmp, sizeof(tmp) - 1);
		if (s == -1) {
			perror("readlink");
			return NULL;
		}

		tmp[s] = 0;
		if (*tmp == '/')
			return path_resolution(tmp, buf, bufsize, flags);
	}

exit:
	return real_path;
}
