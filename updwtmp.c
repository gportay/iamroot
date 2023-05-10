/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <utmp.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void next_updwtmp(const char *path, const struct utmp *ut)
{
	void (*sym)(const char *, const struct utmp *);

	sym = dlsym(RTLD_NEXT, "updwtmp");
	if (!sym)
		return;

	sym(path, ut);
}

void updwtmp(const char *path, const struct utmp *ut)
{
	char buf[PATH_MAX];
	int atflags = 0;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return;

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	next_updwtmp(buf, ut);
}
#endif
