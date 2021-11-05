/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
gid_t next_getgid()
{
	gid_t (*sym)();
	gid_t ret;

	sym = dlsym(RTLD_NEXT, "getegid");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym();
	if (ret == (gid_t)-1)
		__perror(NULL, __func__);

	return ret;
}

gid_t getgid()
{
	unsigned long ul;

	errno = 0;
	ul = strtoul(getenv("IAMROOT_GID") ?: "0", NULL, 0);
	if (!errno)
		return ul;

	__verbose("%s(): IAMROOT_GID: %lu\n", __func__, ul);

	return 0;
}
