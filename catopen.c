/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <nl_types.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
nl_catd next_catopen(const char *path, int flag)
{
	nl_catd (*sym)(const char *, int);
	nl_catd ret;

	sym = dlsym(RTLD_NEXT, "catopen");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return (nl_catd)-1;
	}

	ret = sym(path, flag);
	if (ret == (nl_catd)-1)
		__pathperror(path, __func__);

	return ret;
}

nl_catd catopen(const char *path, int flag)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return (nl_catd)-1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return next_catopen(buf, flag);
}
