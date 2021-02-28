/*
 * Copyright 2020-2021 Gaël PORTAY
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
uid_t next_geteuid()
{
	uid_t (*sym)();

	sym = dlsym(RTLD_NEXT, "geteuid");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym();
}

uid_t geteuid(void)
{
	unsigned long ul;

	errno = 0;
	ul = strtoul(getenv("IAMROOT_GETEUID") ?: "0", NULL, 0);
	if (!errno)
		return ul;

	__verbose("%s(): IAMROOT_GETEUID: %lu\n", __func__, ul);

	return 0;
}
