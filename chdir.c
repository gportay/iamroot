/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_chdir(const char *path)
{
	int (*sym)(const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "chdir");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int chdir(const char *path)
{
	char buf[PATH_MAX];
	int ret;

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	ret = next_chdir(buf);
	if (ret) {
		__pathperror(buf, "chdir");
		return ret;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return chrootdir(NULL);
}
