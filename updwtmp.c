/*
 * Copyright 2023-2024 Gaël PORTAY
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

static void (*sym)(const char *, const struct utmp *);

hidden void next_updwtmp(const char *path, const struct utmp *ut)
{
	if (!sym)
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

	next_updwtmp(buf, ut);

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);
}
#endif
