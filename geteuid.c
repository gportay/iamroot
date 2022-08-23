/*
 * Copyright 2020-2022 Gaël PORTAY
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
uid_t next_geteuid()
{
	uid_t (*sym)();
	uid_t ret;

	sym = dlsym(RTLD_NEXT, "geteuid");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym();
	if (ret == (uid_t)-1)
		__pathperror(NULL, __func__);

	return ret;
}

uid_t geteuid(void)
{
	unsigned long ul;

	errno = 0;
	ul = strtoul(getenv("IAMROOT_EUID") ?: "0", NULL, 0);
	if (errno)
		ul = 0;

	__debug("%s(): IAMROOT_EUID: %lu\n", __func__, ul);

	return ul;
}
