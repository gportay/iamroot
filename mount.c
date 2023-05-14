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
	int ret;
	(void)filesystemtype;
	(void)mountflags;
	(void)source;
	(void)target;
	(void)data;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(source: '%s', target: '%s', ..., mountflags: 0x%lx, ...) -> %i\n",
		__func__, source, target, mountflags, ret);

	return ret;
}
#endif

#if defined __FreeBSD__ || defined __OpenBSD__
int mount(const char *type, const char *dir, int flags, void *data)
{
	int ret;
	(void)flags;
	(void)type;
	(void)data;
	(void)dir;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(..., dir: '%s', flags: 0x%x, ...) -> %i\n", __func__, dir,
		flags, ret);

	return ret;
}
#endif
