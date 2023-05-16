/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

gid_t getgid()
{
	const int save_errno = errno;
	unsigned long ul;

	errno = 0;
	ul = strtoul(__getenv("GID") ?: "0", NULL, 0);
	if (errno)
		ul = 0;

	__debug("%s(): %lu\n", __func__, ul);

	return __set_errno(save_errno, ul);
}
