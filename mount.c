/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/mount.h>

#include "iamroot.h"

int mount(const char *source, const char *target, const char *filesystemtype,
	  unsigned long mountflags, const void *data)
{
	(void)filesystemtype;
	(void)mountflags;
	(void)source;
	(void)target;
	(void)data;

	__verbose("%s(source: '%s', target: '%s', ...)\n", __func__, source,
		  target);

	return 0;
}
