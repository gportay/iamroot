/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

extern char *path_resolution(const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int chrootdir(const char *);

__attribute__((visibility("hidden")))
int next_chdir(const char *path)
{
	int (*sym)(const char *);

	sym = dlsym(RTLD_NEXT, "chdir");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path);
}

int chdir(const char *path)
{
	char buf[PATH_MAX];
	char *real_path;
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = next_chdir(real_path);
	if (ret) {
		perror("chdir");
		return ret;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return chrootdir(NULL);
}
