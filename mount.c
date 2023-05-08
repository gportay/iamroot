/*
 * Copyright 2020-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>
#include <sys/param.h>

#include "iamroot.h"

#ifdef __linux__
int mount(const char *source, const char *target, const char *filesystemtype,
	  unsigned long mountflags, const void *data)
{
	(void)filesystemtype;
	(void)mountflags;
	(void)source;
	(void)target;
	(void)data;

	__debug("%s(source: '%s', target: '%s', ...)\n", __func__, source,
		target);

	return 0;
}
#endif

#if defined __FreeBSD__ || defined __OpenBSD__
int mount(const char *type, const char *dir, int flags, void *data)
{
	(void)flags;
	(void)type;
	(void)data;
	(void)dir;

	__debug("%s(..., dir: '%s', ...)\n", __func__, dir);

	return 0;
}
#endif
