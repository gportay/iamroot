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

#include <utmpx.h>

#include "iamroot.h"

static void (*sym)(const char *, const struct utmpx *);

hidden void next_updwtmpx(const char *path, const struct utmpx *ut)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "updwtmpx");

	if (!sym)
		return;

	sym(path, ut);
}

void updwtmpx(const char *path, const struct utmpx *ut)
{
	char buf[PATH_MAX];
	int atflags = 0;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return;

	next_updwtmpx(buf, ut);

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);
}
#endif
