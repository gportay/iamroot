/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "path_resolution.h"

#define __strlcmp(s1, s2) strncmp(s1, s2, strlen(s2))

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern const char *getrootdir();

__attribute__((visibility("hidden")))
ssize_t next_readlink(const char *path, char *buf, size_t bufsize)
{
	ssize_t (*sym)(const char *, char *, size_t);

	sym = dlsym(RTLD_NEXT, "readlink");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, buf, bufsize);
}

ssize_t readlink(const char *path, char *buf, size_t bufsize)
{
	char tmp[PATH_MAX];
	const char *root;
	char *real_path;
	ssize_t ret;
	size_t len;

	real_path = path_resolution(path, tmp, sizeof(tmp), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = next_readlink(real_path, buf, bufsize);
	if (ret == -1)
		goto exit;

	root = getrootdir();
	if (strncmp(root, "/", ret) == 0)
		goto exit;

	if (__strlcmp(buf, getrootdir()) != 0)
		goto exit;

	len = strlen(root);
	memmove(buf, buf+len, strlen(buf)-len+1);
	ret -= len;

	if (ret == 0)
		buf[ret++] = '/';

exit:
	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return ret;
}
