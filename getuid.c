/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
uid_t next_getuid()
{
	uid_t (*sym)();
	uid_t ret;

	sym = dlsym(RTLD_NEXT, "geteuid");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym();
	if (ret == (uid_t)-1)
		__perror(NULL, __func__);

	return ret;
}

uid_t getuid()
{
	unsigned long ul;

	errno = 0;
	ul = strtoul(getenv("IAMROOT_GETUID") ?: "0", NULL, 0);
	if (!errno)
		return ul;

	__verbose("%s(): IAMROOT_GETUID: %lu\n", __func__, ul);

	return 0;
}
